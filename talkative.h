#pragma once
#include <blah.h>
#include "world.h"
#include <iostream>

using namespace Blah;

namespace BT
{
	class Talkative : public Component
	{
	public:
		void awake() override;
	};
}