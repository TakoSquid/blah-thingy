#pragma once

#include <blah.h>
#include "world.h"
#include "collider.h"
#include "mover.h"
#include <set>

using namespace Blah;

namespace BT
{
	struct Passenger
	{
		Entity* entity;
		Point movement;
		bool standing_on_platform;
		bool move_before_platform;

		Passenger()
			:entity(nullptr),
			movement(Point(0, 0)),
			standing_on_platform(false),
			move_before_platform(false)
		{

		}

		Passenger(Entity* entity, Point movement, bool standing_on_platform, bool move_before_platform)
			:entity(entity),
			movement(movement),
			standing_on_platform(standing_on_platform),
			move_before_platform(move_before_platform)
		{

		}
	};
	
	class MovingPlatform : public Component
	{
	public:
		Collider* collider;

		Vector<Point> checkpoints;


		float speed;

		void update() override;
		void debug() override;
	private:
		Vector<Passenger> passengers;

		Vec2 remainer;
		unsigned int index = 0;
		float t = 0.0f;

		Point velocity_to_display;

		Point calculate_platform_movement();
		void calculate_passenger_movement(Point velocity);
		void move_passengers(bool beforePlatform);
	};
}