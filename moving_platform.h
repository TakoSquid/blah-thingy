#pragma once

#include <blah.h>
#include "world.h"
#include "collider.h"
#include "mover.h"
#include <set>

using namespace Blah;

namespace BT
{
	class MovingPlatform : public Component
	{
	public:
		Collider* collider;
		Vec2 velocity;

		void update() override;
		void debug() override;
	private:
		std::set<Mover*> movers;
		Vec2 remainer;
	};
}