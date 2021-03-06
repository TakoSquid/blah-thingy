#include "mech.h"
#include <iostream>

#include "masks.h"
#include "mover.h"
#include "animator.h"
#include "game.h"

using namespace BT;

namespace
{
	Vec2 offset;
}

Mech::Mech()
{
	input_get_in.press_buffer = .15f;
	input_get_in.add(Button::Y);
	input_get_in.add(Key::W);
}

void BT::Mech::awake()
{
	m_mech_control = get<Player>();
}

void BT::Mech::render(Blah::Batch& batch)
{
	if (m_pilot && m_pilot->get<Animator>()) {
		batch.push_matrix(Mat3x2::create_translation(offset));
		m_pilot->get<Animator>()->render(batch);
		batch.pop_matrix();
	}
}

void BT::Mech::update()
{
	input_get_in.update();

	if (!pick_up_zone->cached() && !m_pilot)
	{
		Entity* potential = nullptr;

		Vector<Collider*> found = pick_up_zone->all(Mask::player);

		for (auto& col : found)
		{
			if (col->entity() != entity() && col->get<Player>())
			{
				potential = col->entity();
				break;
			}
		}

		if (potential)
		{
			if (input_get_in.pressed())
			{
				input_get_in.consume_press();

				set_pilot(potential);
			}

			pick_up_indicator->visible = true;
		}
		else
		{
			pick_up_indicator->visible = false;
		}
	}

	if (m_pilot && input_get_in.pressed())
	{
		input_get_in.consume_press();
		release_pilot();
	}

	if (m_pilot &&  m_pilot->get<Animator>())
	{

		auto animator = m_pilot->get<Animator>();

		int facing = get<Player>()->get_facing();
		animator->scale = Vec2(facing, 1.0f);
		m_pilot->position = entity()->position;

		offset = Vec2(6 * facing, -74);
		float angle = 0;

		if (animator->animation()->name == "run")
		{
			offset += Vec2(facing * 17, 15);
			angle = .7f * facing;
		}
		else if (animator->animation()->name != "idle")
		{
			offset += Vec2(facing * 4, 2);
			angle = .2f * facing;
		}

		m_pilot->rotation = angle;

		pick_up_indicator->visible = false;
	}

	auto mover = get<Mover>();
	
	if (mover)
	{
		if (!grounded_last_frame && mover->on_ground() && mover->speed.y >= 0)
		{
			stomp_effects();
		}

		grounded_last_frame = mover->on_ground();
	}

	// Shake camera when foot down

	auto animator = get<Animator>();

	if (animator)
	{
		int current_frame = animator->get_current_frame_index();

		if (animator->animation()->name == "run" && (current_frame == 3 || current_frame == 14))
		{
			if (!stop_already) {
				stomp_effects();
				stop_already = true;
			}
		}
		else {
			stop_already = false;
		}

	}
}

void Mech::set_pilot(Entity* pilot)
{
	m_mech_control->active = true;
	m_mech_control->get<Mover>()->gravity = 0;
	m_mech_control->get<Mover>()->friction = 0;

	m_pilot = pilot;
	m_pilot->active = false;
	m_pilot->visible = false;

	if(auto anim = m_pilot->get<Animator>())
		anim->play("idle");
}

void Mech::release_pilot()
{
	m_pilot->position = entity()->position + Point(0, -78);
	m_pilot->visible = true;
	m_pilot->active = true;
	m_pilot->get<Mover>()->speed = m_mech_control->get<Mover>()->speed;
	m_pilot->rotation = 0;

	m_pilot = nullptr;

	m_mech_control->get<Mover>()->gravity = 450;
	m_mech_control->get<Mover>()->friction = 500;
	m_mech_control->get<Animator>()->play("idle");

	m_mech_control->active = false;
}

void Mech::stomp_effects()
{
	world()->game->camera.shake();
	Content::play_sound("stomp");
}
