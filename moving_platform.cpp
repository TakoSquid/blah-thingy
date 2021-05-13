#include "moving_platform.h"

namespace BT
{
	void MovingPlatform::update()
	{
		if (velocity != Point(0, 0))
		{
			//entity()->position = entity()->position + velocity * Time::delta;

			auto next = Calc::approach(entity()->position, entity()->position + Point(0, -114), 20 * Time::delta);

			entity()->position = Point(next.x, next.y);
		}

	}
}