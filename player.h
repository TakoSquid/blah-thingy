#pragma once
#include "world.h"
#include <blah.h>

using namespace Blah;

namespace BT
{
	class Collider;
	class Player : public Component
	{
	public:
		int st_normal = 0;
		int st_attack = 1;
		int st_hurt = 2;
		int st_start = 3;
		int max_health = 4;

		float max_ground_speed = 100;
		float max_air_speed = 90;
		float max_speed_reducer = 750;
		float ground_accel = 500;
		float air_accel = 200;
		float friction = 500;
		float hurt_friction = 200;
		float gravity = 450;
		float jump_force = -120;
		float jump_time = 0.18f;
		float hurt_duration = 0.5f;
		float invincible_duration = 1.5f;
		float slope_mult_max = 1.0f;
		float slope_mult_min = 1.0f;

		int health = max_health;

		StickBinding input_move;
		Binding input_jump;
		Binding input_attack;

		Player();
		void update() override;

		const int get_facing() const;

	private:
		int m_state = st_normal;
		int m_facing = 1;
		float m_jump_timer = 0;
		float m_attack_timer = 0;
		float m_hurt_timer = 0;
		float m_invincible_timer = 0;
		float m_start_timer = 1;
		Collider* m_attack_collider = nullptr;
		bool m_on_ground = false;
		Collider* m_on_slope = nullptr;
	};
}