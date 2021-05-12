#pragma once
#include "world.h"
#include "collider.h"
#include <blah.h>
#include <functional>

using namespace Blah;

namespace BT
{
	class Mover : public Component
	{
	private:
		Vec2 m_remainer;

	public:
		Collider* collider = nullptr;
		Collider* slope_collider = nullptr;
		Collider* jumpthru_collider = nullptr;

		Vec2 speed;
		float gravity = 0;
		float friction = 0;

		std::function<void(Mover*)> on_hit_x;
		std::function<void(Mover*)> on_hit_y;

		bool move_x(int amount);
		bool move_y(int amount);

		void stop_x();
		void stop_y();
		void stop();

		bool on_ground(int dist = 1) const;
		Collider* on_slope(int dist = 1) const;

		void update() override;
	};
}