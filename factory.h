#pragma once
#include <blah.h>
#include "world.h"

#include "masks.h"

using namespace Blah;

namespace BT
{
	namespace Factory
	{
		Entity* player(World* world, Point point);
		Entity* debugSolid(World* world, Point pos, Point size, uint32_t mask = Mask::solid);
		Entity* mech(World* world, Point point);
		Entity* button(World* world, Point point);
	}
}