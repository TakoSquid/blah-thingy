#pragma once
#include <blah.h>
#include "world.h"

using namespace Blah;

namespace BT
{
	class DebugSolid : public Component
	{
	public:
		void render(Batch& batch) override;
	};
}