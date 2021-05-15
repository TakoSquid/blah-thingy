#include "content.h"
#include "game.h"
#include "sprite.h"
#include "tileset.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace BT;

// WARN: need to read more about static non const stuff
irrklang::ISoundEngine* Content::sound_engine;
Blah::Vector<SoundInfo*> Content::sounds;

namespace
{
	struct SpriteInfo
	{
		String name;
		Aseprite asesprite;
		uint64_t pack_index;
	};

	FilePath root;
	Vector<Sprite> sprites;
	Vector<Tileset> tilesets;
	Vector<Subtexture> subtextures;
	Vector<RoomInfo> rooms;
	TextureRef sprite_atlas;
}

SpriteFont Content::font;

FilePath Content::path()
{
	if (root.length() <= 0)
	{
		FilePath up = "";

		do
		{
			root = Path::normalize(FilePath::fmt("%s/%scontent/", App::path(), up.cstr()));
			up.append("../");
		} while (!Directory::exists(root) && up.length() < 30);

		if (!Directory::exists(root))
			BLAH_ERROR("Unable de find content directory !");

		Log::info("Content Path: %s", root.cstr());
	}

	return root;
}

void Content::load()
{
	Packer packer;
	packer.padding = 0;

	// font
	font = SpriteFont(path() + "fonts/Early GameBoy.ttf", 8);
	font.line_gap = 4;

	// sprites
	Vector<SpriteInfo> sprite_info;
	uint64_t pack_index = 0;

	{
		FilePath sprite_path = path() + "sprites/";

		for (auto& it : Directory::enumerate(sprite_path, true))
		{
			if (!it.ends_with(".ase"))
				continue;

			SpriteInfo* info = sprite_info.expand();
			info->asesprite = Aseprite(it.cstr());
			info->name = String(it.cstr() + sprite_path.length(), it.end() - 4);
			info->pack_index = pack_index;
			for (auto& frame : info->asesprite.frames)
			{
				packer.add(pack_index, frame.image);
				pack_index++;
			}
		}
	}

	// sounds & musics
	{
		sound_engine = irrklang::createIrrKlangDevice();
		sound_engine->setSoundVolume(.1);

		if (!sound_engine)
			Log::error("Sound engine couldn't start !");

		FilePath sound_path = path() + "sounds/";

		for (auto& it : Directory::enumerate(sound_path, true))
		{
			if (!it.ends_with(".wav"))
				continue;

			SoundInfo* sound_info = new SoundInfo();

			sound_info->sound_source = sound_engine->addSoundSourceFromFile(it.cstr(), ESM_AUTO_DETECT, true);
			sound_info->name = String(it.cstr() + sound_path.length(), it.end() - 4);

			sounds.push_back(sound_info);
		}
	}

	// tilesets
	Vector<SpriteInfo> tileset_info;
	{
		FilePath sprite_path = path() + "tilesets/";
		for (auto& it : Directory::enumerate(sprite_path, true))
		{
			if (!it.ends_with(".ase"))
				continue;

			SpriteInfo* info = tileset_info.expand();
			info->asesprite = Aseprite(it.cstr());
			info->name = String(it.cstr() + sprite_path.length(), it.end() - 4);
			info->pack_index = pack_index;

			auto& frame = info->asesprite.frames[0];
			auto columns = frame.image.width / Game::tile_width;
			auto rows = frame.image.height / Game::tile_height;

			for (int x = 0; x < columns; x++)
				for (int y = 0; y < rows; y++)
				{
					auto subrect = RectI(x * Game::tile_width, y * Game::tile_height, Game::tile_width, Game::tile_height);
					auto subimage = frame.image.get_sub_image(subrect);
					packer.add(pack_index, subimage);
					pack_index++;
				}
		}
	}

	// build atlas
	{
		packer.pack();
		sprite_atlas = Texture::create(packer.pages[0]);

		subtextures.expand(packer.entries().size());
		for (auto& entry : packer.entries())
			subtextures[entry.id] = Subtexture(sprite_atlas, entry.packed, entry.frame);
	}

	// add sprites
	for (auto& info : sprite_info)
	{
		Sprite sprite;
		sprite.name = info.name;
		sprite.origin = Vec2::zero;

		if (info.asesprite.slices.size() > 0 && info.asesprite.slices[0].has_pivot)
		{
			sprite.origin = Vec2(
				info.asesprite.slices[0].pivot.x,
				info.asesprite.slices[0].pivot.y
			);
		}

		for (auto& tag : info.asesprite.tags)
		{
			Sprite::Animation anim;
			anim.name = tag.name;

			for (int i = tag.from; i <= tag.to; i++)
			{
				Sprite::Frame frame;
				frame.duration = info.asesprite.frames[i].duration / 1000.0f;
				frame.image = subtextures[info.pack_index + i];
				anim.frames.push_back(frame);
			}
			sprite.animations.push_back(anim);
		}
		sprites.push_back(sprite);
	}

	// add tilesets
	for (auto& info : tileset_info)
	{
		auto& frame = info.asesprite.frames[0];

		Tileset* tileset = tilesets.expand();
		tileset->name = info.name;
		tileset->columns = frame.image.width / Game::tile_width;
		tileset->rows = frame.image.height / Game::tile_height;

		for (int x = 0, i = info.pack_index; x < tileset->columns; x++)
			for (int y = 0; y < tileset->rows; y++)
			{
				tileset->tiles[x + y * tileset->columns] = subtextures[i];
				i++;
			}
	}

	//load the rooms
	for (auto& it : Directory::enumerate(path() + "/map", false))
	{
		if (!it.ends_with(".json"))
			continue;

		auto name = Path::get_file_name_no_ext(it);
		auto point = name.split('x');
		if (point.size() != 2)
			continue;

		RoomInfo info;
		info.name = name;
		//info.cell.x = strtol(point[0].cstr(), nullptr, 0);
		//info.cell.y = strtol(point[1].cstr(), nullptr, 0);

		json j;
		std::ifstream i(it);
		i >> j;

		info.offset = Point(j["offsetX"].get<int>(), j["offsetY"].get<int>());
		info.size = Point(j["width"].get<int>(), j["height"].get<int>());

		for (auto& it : j["layers"])
		{
			if (it["name"] != "entities")
			{
				//BLAH_ASSERT(it["gridCellsX"] == Game::columns, "layer has incorrect width");
				//BLAH_ASSERT(it["gridCellsY"] == Game::rows, "layer has incorrect width");

				TileLayer layer;

				layer.name = it["name"].get<std::string>().c_str();
				layer.tileset = it["tileset"].get<std::string>().c_str();
				layer.column = it["gridCellsX"].get<int>();
				layer.row = it["gridCellsY"].get<int>();

				for (auto& it : it["data"].get<std::vector<int>>())
				{
					layer.data.emplace_back(it);
				}

				info.layers.push_back(layer);
			}
			else
			{
				ParsedEntity parsedEntity;

				for (auto& it : it["entities"])
				{
					parsedEntity.name = it["name"].get<std::string>().c_str();
					parsedEntity.position.x = it["x"].get<int>();
					parsedEntity.position.y = it["y"].get<int>();
					parsedEntity.id = it["id"].get<int>();

					if (!it["values"].is_null())
					{
						parsedEntity.values = it["values"];
					}

					/*if(it["values"])
						parsedEntity.values = it["values"];*/

					info.entities.push_back(parsedEntity);
				}
			}
		}

		rooms.push_back(info);
	}
}

void Content::unload()
{
	font.dispose();
	sound_engine->drop();
}

TextureRef Content::atlas()
{
	return sprite_atlas;
}

const Sprite* Content::find_sprite(const char* name)
{
	for (auto& it : sprites)
		if (it.name == name)
			return &it;

	return nullptr;
}

const Tileset* Content::find_tileset(const char* name)
{
	for (auto& it : tilesets)
		if (it.name == name)
			return &it;

	return nullptr;
}

//const RoomInfo* Content::find_room(const Point& cell)
//{
//	for (auto& it : rooms)
//		if (it.cell == cell)
//			return &it;
//
//	return nullptr;
//}

const RoomInfo* Content::find_room(const String& name)
{
	for (auto& it : rooms)
		if (it.name == name)
			return &it;

	Log::error("SMTH WENT WRONG FK");

	return nullptr;
}

const RoomInfo* Content::find_room_by_pos(const Point& position)
{
	for (auto& it : rooms)
		if (RectI(it.offset, it.size).contains(position))
			return &it;

	return nullptr;
}

void Content::play_sound(const char* name, bool playLooped, float playbackSpeed)
{
	for (auto& it : sounds)
		if (it->name == name) {
			ISound* snd = sound_engine->play2D(it->sound_source, playLooped, true, true, false);
			if (snd)
			{
				snd->setPlaybackSpeed(playbackSpeed);
				snd->setIsPaused(false);
			}
		}
}