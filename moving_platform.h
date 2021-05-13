#pragma once

#include <blah.h>
#include "world.h"
#include "collider.h"

using namespace Blah;

namespace BT
{
	class MovingPlatform : public Component
	{
	public:
		Collider* collider;
		Point velocity;

		void update() override;
	private:
	};
}