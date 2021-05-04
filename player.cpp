#include "player.h"
#include "mover.h"
#include "animator.h"
#include "collider.h"
#include "content.h"

#include "masks.h"

#include <iostream>

#include <irrKlang.h>

using namespace BT;

namespace
{
}

Player::Player()
{
	input_move.add_dpad(0);
	input_move.add_left_stick(0, .2f);
	input_move.add(Key::Left, Key::Right, Key::Up, Key::Down);

	input_jump.press_buffer = .15f;
	input_jump.add(Key::X, Button::A);

	input_attack.press_buffer = .15f;
	input_attack.add(Key::C, Button::X);
}

void Player::update()
{
	input_move.update();
	input_jump.update();
	input_attack.update();

	auto mover = get<Mover>();
	auto anim = get<Animator>();
	auto hitbox = get<Collider>();

	bool was_on_ground = m_on_ground;
	m_on_ground = mover->on_ground();
	m_on_slope = mover->on_slope();

	int input = input_move.sign().x;

	// Sprite
	{
		if (anim) {
		if (!was_on_ground && m_on_ground && mover->speed.y >= 0)
			anim->scale = Vec2(m_facing * 1.5f, 0.7f);

		
			anim->scale = Calc::approach(anim->scale, Vec2(m_facing, 1.0f), Time::delta * 4);

			anim->scale.x = Calc::abs(anim->scale.x) * m_facing;
		}
	}

	if (m_state == st_normal)
	{
		if (anim) {
			if (m_on_ground && mover->speed.y >= 0)
			{
				if (input != 0)
					anim->play("run");
				else
					anim->play("idle");
			}
			else
			{
				if (mover->speed.y < 0)
					anim->play("jump");
				else if (Calc::abs(mover->speed.y) <= 50.0f)
					anim->play("mid-air");
				else
					anim->play("fall");
			}
		}

		// x axis

		mover->speed.x += input_move.value().x * (m_on_ground ? ground_accel : air_accel) * Time::delta;

		Collider::Slope slope;
		float slope_mult = 1.0f;

		if (m_on_slope)
		{
			slope = m_on_slope->get_slope();
			slope_mult = slope.get_speed_mult(slope.facing != m_facing);
			slope_mult = Calc::clamp(slope_mult, slope_mult_min, slope_mult_max);
		}

		auto max_speed = (m_on_ground ? (m_on_slope ? max_ground_speed * slope_mult : max_ground_speed) : max_air_speed);
		if (Calc::abs(mover->speed.x) > max_speed)
		{
			mover->speed.x = Calc::approach(mover->speed.x, Calc::sign(mover->speed.x) * max_speed, max_speed_reducer * Time::delta);
		}

		if (input == 0 && m_on_ground)
			mover->speed.x = Calc::approach(mover->speed.x, 0, friction * Time::delta);

		if (input != 0 && m_on_ground)
			m_facing = input;

		// Jump press
		if (input_jump.pressed() && mover->on_ground())
		{
			input_jump.consume_press();
			if(anim)
				anim->scale = Vec2(m_facing * 0.65f, 1.4f);
			mover->speed.x = input_move.value().x * max_air_speed;
			m_jump_timer = jump_time;

			// TODO: switch to Content (using folder() )
			Content::play_sound("jump");
		}
	}

	if (m_jump_timer > 0)
	{
		mover->speed.y = jump_force;
		m_jump_timer -= Time::delta;
		if (!input_jump.down())
			m_jump_timer = 0;
	}

	if (!m_on_ground)
	{
		float grav = gravity;
		if (m_state == st_normal && Calc::abs(mover->speed.y) < 20 && input_jump.down())
			grav *= 0.4;

		mover->speed.y += grav * Time::delta;
	}
}

const int BT::Player::get_facing() const
{
	return m_facing;
}