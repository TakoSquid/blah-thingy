#include "game.h"
#include "content.h"
#include "collider.h"
#include "masks.h"
#include "tilemap.h"
#include "tileset.h"
#include "player.h"
#include "mech.h"
#include "button.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "dear_imgui.h"
#include "world.h"
#include "animator.h"
#include "moving_platform.h"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace BT {
	namespace {
		Point player_pos;
		int display_scale = 3;
		TextureRef game_texture = nullptr;

		static Entity* selected = nullptr;
		static Entity* mouse_entity = nullptr;

		TextureRef up_arrow;
		TextureRef right_arrow;
		TextureRef pivot;
		const int pivot_radius = 2;
	}

	void Game::startup() {
		world.game = this;

		Content::load();

		room = Content::find_room("1x0");

		camera = Camera();

		camera.set_target_box(Rect(room->offset, Vec2(50, 50)));
		camera.using_target_box = true;

		camera.set_allowed_area(Rect(room->offset, room->size));
		camera.using_allowed_area = true;

		buffer = FrameBuffer::create(width, height);

		batch.default_sampler = TextureSampler(TextureFilter::Nearest);
		m_draw_colliders = false;

		m_draw_solid = false;
		m_draw_jumpthrough = false;
		m_draw_slope = false;

		load_room("1x0");
		load_room("2x0");
		load_room("-1x0");
		load_room("0x0");
		load_room("2x1");

		mouse_entity = world.add_entity();
		mouse_entity->add<Collider>(Collider::make_rect(RectI(Point::zero, Point::one)));

		up_arrow = Texture::create(Content::path() + "images/up_arrow.png");
		right_arrow = Texture::create(Content::path() + "images/right_arrow.png");
		pivot = Texture::create(Content::path() + "images/pivot.png");
	}

	void Game::load_room(String name, bool is_reload)
	{
		const RoomInfo* roomInfo = Content::find_room(name);
		BLAH_ASSERT(roomInfo, "Room doesn't exists !");

		for (auto& it : roomInfo->layers)
		{
			// solid
			{
				if (it.name == "solid")
				{
					auto en = world.add_entity(roomInfo->offset);
					en->name = roomInfo->name + " : solids";
					auto solid = en->add(Collider::make_grid(16, it.column, it.row));
					solid->mask = Mask::solid;

					for (std::size_t i = 0; i != it.data.size(); ++i) {
						if (it.data[i] == -1)
							continue;
						int x = i % it.column;
						int y = Calc::floor(i / it.column);

						solid->set_cell(x, y, true);
					}
				}
			}

			// jumpthru
			{
				if (it.name == "jumpthru")
				{
					// TODO: Care ! 16*16 in Ogmo transformed in 8*8 here

					auto en = world.add_entity(roomInfo->offset);
					en->name = roomInfo->name + " : jumptru";
					auto jumpthru = en->add(Collider::make_grid(8, it.column * 2, it.row * 2));
					jumpthru->mask = Mask::jumpthru;

					for (std::size_t i = 0; i != it.data.size(); ++i) {
						if (it.data[i] == -1)
							continue;

						int x = i % it.column;
						int y = Calc::floor(i / it.column);

						jumpthru->set_cell(x * 2, y * 2, true);
						jumpthru->set_cell(x * 2 + 1, y * 2, true);
					}
				}
			}

			// slope
			{
				if (it.name == "slope")
				{
					for (std::size_t i = 0; i != it.data.size(); ++i)
					{
						int id = it.data[i];

						if (id == -1)
							continue;
						int x = i % it.column;
						int y = Calc::floor(i / it.column);

						Point top_left = Point(x * tile_width, y * tile_height);

						int facing = 1;
						if (id == 0 || id == 2 || id == 3)
							facing = -1;

						Point y_offset = Point();
						int height = 16;

						if (id == 2 || id == 5)
						{
							y_offset.y = 8;
							height = 8;
						}

						if (id == 3 || id == 4)
						{
							height = 8;
						}

						auto en = world.add_entity(roomInfo->offset);
						en->name = roomInfo->name + " : slope";
						auto slope = en->add(Collider::make_slope(RectI(top_left + y_offset, Point(16, height)), facing));

						slope->mask = Mask::slope;
					}
				}
			}

			// scenery
			{
				if (it.name == "foreground" || it.name == "background")
				{
					auto en = world.add_entity(roomInfo->offset);
					en->name = roomInfo->name + " : " + it.name;
					auto tilemap = en->add(Tilemap(16, 16, it.column, it.row));
					auto tileset = Content::find_tileset(it.tileset);

					for (std::size_t i = 0; i != it.data.size(); ++i) {
						if (it.data[i] == -1)
							continue;
						int x = i % it.column;
						int y = Calc::floor(i / it.column);

						tilemap->set_cell(x, y, &tileset->tiles[it.data[i]]);
					}

					if (it.name == "foreground")
					{
						tilemap->depth = -1;
					}

					if (it.name == "background")
					{
						tilemap->depth = 1;
					}
				}
			}
		}

		// instantiate parsed entities

		Vector<Entity*> entity_room; // Entities being loaded, dropped when room is loaded

		{
			for (auto& it : roomInfo->entities)
			{
				Entity* en = nullptr;

				if (it.listeners.size() != 0) {
					Log::info("%s has %d listeners !", it.name.cstr(), it.listeners.size());

					for (const auto& list : it.listeners)
					{
						Log::info("%d", list);
					}
				}

				if (it.name == "player")
				{
					if (!world.first<Player>())
						en = Factory::player(&world, it.position + roomInfo->offset);
				}

				if (it.name == "mech")
				{
					if (!world.first<Mech>())
						en = Factory::mech(&world, it.position + roomInfo->offset);
				}

				if (it.name == "button")
				{
					en = Factory::button(&world, it.position + roomInfo->offset);
				}

				if (it.name == "moving platform")
				{
					Vector<Point> checkpoints;

					checkpoints.emplace_back(it.position + roomInfo->offset);

					for (const auto& elem : it.values["nodes"])
					{
						checkpoints.emplace_back(elem["x"].get<int>() + roomInfo->offset.x, elem["y"].get<int>() + roomInfo->offset.y);
					}

					en = Factory::moving_platform(&world, it.position + roomInfo->offset, it.values["speed"], checkpoints);

				}

				if (en)
				{
					en->ogmoId = it.id;
					entity_room.emplace_back(en);
				}
			}
		}

		// Links terrible implementation, I want to burry this code
		{
			for (auto& it : roomInfo->entities)
			{
				if (it.listeners.size() == 0)
					continue;

				Entity* target = nullptr;
				Vector<Entity*> listeners;

				for (auto& ent : entity_room)
				{
					if (ent->ogmoId == it.id)
					{
						target = ent;
						break;
					}
				}

				if (target) {
					Log::info("Concerned : %s", target->name.cstr());
				}
				else
				{
					continue;
				}

				for (auto& ent : entity_room)
				{
					for (const auto& list_id : it.listeners)
					{
						if (ent->ogmoId == list_id) {
							listeners.emplace_back(ent);
							Log::info("Listener : %s", ent->name.cstr());
						}
					}
				}

				auto sb_target = target->get<SignalBox>();

				for (const auto& list : listeners)
				{
					sb_target->addSignalBox(list->get<SignalBox>());
				}
			}
		}

		// ???
		DearImgui::game = buffer->attachment(0);
	}

	void Game::shutdown() {
		Content::unload();
	}

	void Game::update() {
		static bool paused = false;

		// Play pause window
		{
			ImGui::Begin("CHAOS CONTROL", 0, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::Button(paused ? "Play" : "Pause"))
				paused = !paused;
			ImGui::End();
		}

		if (!paused) {
			if (auto player = world.first<Player>())
				player_pos = player->entity()->position;

			if (Input::pressed(Key::F2))
			{
				world.clear();
				load_room(room->name);
			}

			if (!camera.is_in_transition())
				world.update();

			if (auto player = world.first<Player>()) {
				player_pos = player->entity()->position;
				camera.update_rect_target(player->get<Collider>()->get_rect() + player->entity()->position);
			}

			//camera.set_position(Vec2(player_pos) - camera.get_size() / 2.0f);

			if (!RectI(room->offset, room->size).contains(player_pos) || !room)
			{
				if (auto found_room = Content::find_room_by_pos(player_pos))
				{
					room = found_room;

					auto allowed_area = Rect(found_room->offset, found_room->size);
					auto camera_pos = camera.get_position();
					camera.set_size(Vec2(room->cam_size));

					auto target_x = Calc::clamp((float)player_pos.x - camera.get_size().x/2, allowed_area.left(), allowed_area.right() - camera.get_size().x);
					auto target_y = Calc::clamp((float)player_pos.y - camera.get_size().y/2, allowed_area.top(), allowed_area.bottom() - camera.get_size().y);

					camera.lerp_to(Vec2(target_x, target_y), room->cam_scale);
					
					camera.set_scale(room->cam_scale);
					camera.set_allowed_area(allowed_area);
					camera.using_allowed_area = true;
				}
				else
				{
					Log::warn("Player outside of every room, not supposed to happen !");
				}
			}




			camera.update();
		}

		// ImGui drawing
		{
			ImGuiIO& io = ImGui::GetIO();
			ImVec2 pos = ImGui::GetCursorPos(); //NOT GetCursorScreenPos() -> 1h of my life I wont get back
			static Point pixel_hovered = Point::zero;
			static Point ingame_position = Point::zero;

			// Game and Debug Display TODO : untangle
			{
				ImGui::Begin("Game", 0, ImGuiWindowFlags_AlwaysAutoResize);

				if (game_texture) {
					ImVec2 win_pos = ImGui::GetWindowPos();

					DearImgui::display_texture(game_texture, Vec2(game_texture->width(), game_texture->height()) * display_scale, Vec2(0, 1), Vec2(1, 0));

					if (ImGui::IsItemHovered())
					{
						pixel_hovered.x = (int)((io.MousePos.x - (pos.x + win_pos.x)) / display_scale);
						pixel_hovered.y = (int)((io.MousePos.y - (pos.y + win_pos.y)) / display_scale);

						ingame_position = Point(pixel_hovered.x + (int)camera.get_position().x, pixel_hovered.y + (int)camera.get_position().y);

						ImGui::BeginTooltip();
						ImGui::Text("mouse pos : {%d:%d}", (int)io.MousePos.x, (int)io.MousePos.y);
						ImGui::Text("cursor pos : {%d:%d}", (int)pos.x, (int)pos.y);
						ImGui::Text("win pos : {%d:%d}", (int)win_pos.x, (int)win_pos.y);
						ImGui::Text("pixel hovered : {%d:%d}", pixel_hovered.x, pixel_hovered.y);
						ImGui::Text("in game : {%d:%d}", ingame_position.x, ingame_position.y);

						if (ImGui::IsMouseClicked(0))
						{
							Vector<Entity*> found_entities;
							mouse_entity->position = ingame_position;
							Collider* mouse_col = mouse_entity->get<Collider>();
							found_entities.clear();

							auto collider = world.first<Collider>();
							while (collider)
							{
								if (collider == mouse_col) {
									collider = (Collider*)collider->next();
									continue;
								}

								if (mouse_col->overlaps(collider))
								{
									found_entities.emplace_back(collider->entity());
								}

								collider = (Collider*)collider->next();
							}

							if (found_entities.size() > 0)
								selected = found_entities.front();
						}
						ImGui::EndTooltip();
					}

					if (selected)
					{
						static Vec2 selected_cursorpos;

						if (selected_cursorpos.x + right_arrow->width() <= width * display_scale && selected_cursorpos.y <= height * display_scale)
						{
							static Point mouse_pivot_offset;

							ImGui::SetCursorPos(ImVec2(selected_cursorpos.x, selected_cursorpos.y - right_arrow->height() / 2));
							DearImgui::display_texture(right_arrow, Vec2(right_arrow->width(), right_arrow->height()));

							static bool right_arrow_active = false;
							if (right_arrow_active)
							{
								selected->position = Point(ingame_position.x, selected->position.y) - mouse_pivot_offset;
							}

							if (ImGui::IsItemClicked() || (right_arrow_active && !ImGui::IsItemClicked() && ImGui::IsMouseClicked(0)))
							{
								right_arrow_active = !right_arrow_active;
								if (right_arrow_active)
									mouse_pivot_offset = Point((ingame_position - selected->position).x, 0);
							}

							ImGui::SetCursorPos(ImVec2(selected_cursorpos.x - up_arrow->width() / 2, selected_cursorpos.y - up_arrow->height()));
							DearImgui::display_texture(up_arrow, Vec2(up_arrow->width(), up_arrow->height()));

							static bool up_arrow_active = false;
							if (up_arrow_active)
							{
								selected->position = Point(selected->position.x, ingame_position.y) - mouse_pivot_offset;
							}

							if (ImGui::IsItemClicked() || (up_arrow_active && !ImGui::IsItemClicked() && ImGui::IsMouseClicked(0)))
							{
								up_arrow_active = !up_arrow_active;
								if (up_arrow_active)
									mouse_pivot_offset = Point(0, (ingame_position - selected->position).y);
							}

							ImGui::SetCursorPos(ImVec2(selected_cursorpos.x - pivot->width() / 2, selected_cursorpos.y - pivot->height() / 2));
							DearImgui::display_texture(pivot, Vec2(pivot->width(), pivot->height()));
						}

						selected_cursorpos = (Vec2(selected->position) - camera.get_position()) * display_scale + Vec2(pos.x, pos.y);
					}
				}

				ImGui::End();
			}

			// Check boxes for display options
			{
				ImGui::Begin("Display option", 0, ImGuiWindowFlags_AlwaysAutoResize);
				ImGui::PushItemWidth(100);
				ImGui::SliderInt("Scale", &display_scale, 1, 5);
				ImGui::Checkbox("Draw pivots", &m_draw_pivot);
				ImGui::Checkbox("Draw colliders", &m_draw_colliders);
				ImGui::Checkbox("Draw solids", &m_draw_solid);
				ImGui::Checkbox("Draw jumpthroughs", &m_draw_jumpthrough);
				ImGui::Checkbox("Draw slopes", &m_draw_slope);
				ImGui::Checkbox("Draw signal boxes", &m_draw_sb);
				ImGui::Checkbox("Draw cam info", &m_draw_cam);
				ImGui::End();
			}

			// Com Tree
			{
				ImGui::Begin("COM Tree", 0, ImGuiWindowFlags_AlwaysAutoResize);

				auto entity = world.first_entity();
				while (entity)
				{
					auto next = entity->next();
					auto components = entity->components();

					ImGuiTreeNodeFlags node_flags = (entity == selected) ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;

					bool tree_open = ImGui::TreeNodeEx((void*)entity, node_flags, "%s", entity->name.cstr());

					if (ImGui::IsItemClicked())
						selected = entity;

					if (tree_open) {
						for (const auto& c : components)
						{
							ImGui::Text("%s", c->name().cstr());
						}

						ImGui::TreePop();
					}

					entity = next;
				}

				ImGui::End();
			}

			// Entity selected
			{
				ImGui::Begin("Entity properties", 0, ImGuiWindowFlags_AlwaysAutoResize);
				if (selected)
				{
					ImGui::AlignTextToFramePadding();
					ImGui::Text(selected->name.cstr());
					ImGui::SameLine(ImGui::GetWindowWidth() - 25);
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0.75f, 0.0f, 0.0f)));
					bool e_press_delete = ImGui::Button("x");
					ImGui::PopStyleColor();

					ImGui::Checkbox("Visible", &(selected->visible));
					ImGui::SameLine();
					ImGui::Checkbox("Active", &(selected->active));

					int pos[2] = { selected->position.x , selected->position.y };
					ImGui::InputInt2("Position", pos);
					selected->position = Point(pos[0], pos[1]);
					int scale[2] = { selected->scale.x , selected->scale.y };
					ImGui::InputInt2("Scale", scale);
					selected->scale = Point(scale[0], scale[1]);
					ImGui::SliderFloat("Rotation", &selected->rotation, 0, Calc::PI * 2);

					for (auto& c : selected->components())
					{
						ImGui::Separator();

						std::stringstream ss;
						ss << c->name().cstr() << "##" << &c;

						ImGui::AlignTextToFramePadding();
						//bool tree_open = ImGui::TreeNodeEx((void*)entity, node_flags, "%s", entity->name.cstr());
						bool component_open = ImGui::TreeNodeEx(ss.str().c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);

						ss.str("");
						ss << "x##" << &c;
						ImGui::SameLine(ImGui::GetWindowWidth() - 25);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(0.75f, 0.0f, 0.0f)));
						bool c_press_delete = ImGui::Button(ss.str().c_str());
						ImGui::PopStyleColor();

						if (component_open)
						{
							ss.str("");
							ss << "visible##" << &c;
							ImGui::Checkbox(ss.str().c_str(), &(c->visible));
							ImGui::SameLine();
							ss.str("");
							ss << "active##" << &c;
							ImGui::Checkbox(ss.str().c_str(), &(c->active));
							ImGui::SameLine();
							ss.str("");
							ss << "depth##" << &c;
							ImGui::PushItemWidth(80);
							ImGui::InputInt(ss.str().c_str(), &(c->depth));
							ImGui::Separator();
							c->debug();
							ImGui::TreePop();
						}

						if (c_press_delete)
						{
							c->destroy();
						}
					}

					if (e_press_delete)
					{
						selected->destroy();
						selected = nullptr;
					}
				}
				ImGui::End();
			}
		}
	}

	void Game::render()
	{
		// Draw gameplay to buffer
		{

			buffer->clear(Color(40, 40, 40));

			batch.push_matrix(camera.get_matrix());

			world.render(batch);

			// draw debug colliders

			{
				if (m_draw_colliders)
				{
					auto collider = world.first<Collider>();
					while (collider)
					{
						collider->render(batch);
						collider = (Collider*)collider->next();
					}
				}
				else
				{
					auto collider = world.first<Collider>();
					while (collider)
					{
						if (m_draw_solid && collider->mask == Mask::solid)
							collider->render(batch);

						if (m_draw_jumpthrough && collider->mask == Mask::jumpthru)
							collider->render(batch);

						if (m_draw_slope && collider->mask == Mask::slope)
							collider->render(batch);

						collider = (Collider*)collider->next();
					}
				}
			}

			// draw debug cam

			{
				if (m_draw_cam)
				{
					batch.rect_line(Rect(camera.get_position(), camera.get_size()), 1, Color::teal);
					batch.rect_line(camera.get_allowed_area(), 1, Color::green);
					batch.rect_line(camera.get_target_box(), 1, Color::yellow);
				}
			}

			// Drawing arrow for signal boxes
			{
				if (m_draw_sb)
				{
					auto sb = world.first<SignalBox>();
					while (sb)
					{
						for (const auto& targets : sb->get_listeners()) {
							batch.line(sb->entity()->position, targets->entity()->position, 1.0f, Color::red);
							batch.arrow_head(targets->entity()->position, sb->entity()->position, 6.0f, Color::red);
						}

						sb = (SignalBox*)sb->next();
					}
				}
			}

			batch.pop_matrix();

			batch.render(buffer);

			batch.clear();
		}

		// Draw buffer to screen
		{
			//float scale = Calc::min(App::backbuffer->width() / (float)buffer->width(), App::backbuffer->height() / (float)buffer->height());

			Vec2 screen_center = Vec2(App::backbuffer->width(), App::backbuffer->height()) / 2;
			Vec2 buffer_center = Vec2(buffer->width(), buffer->height()) / 2;

			App::backbuffer->clear(Color::black);
			batch.push_matrix(Mat3x2::create_transform(screen_center, buffer_center, Vec2::one * display_scale, 0));

			game_texture = buffer->attachment(0);

			batch.tex(game_texture, Vec2::zero, Color::white);
			batch.pop_matrix();
			//batch.render(App::backbuffer);
			batch.clear();
		}
	}
}