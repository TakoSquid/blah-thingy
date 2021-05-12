#pragma once
#include <blah.h>
#include <functional>
#include "world.h"
#include "collider.h"
#include "signal_box.h"

using namespace Blah;

namespace BT
{
	class Button : public Component
	{
	public:
		Button(bool pressed = false);

		void update() override;

		Collider* trigger_zone;
		SignalBox* signal_box;
	private:
		bool m_pressed;
		bool m_pressed_old;
		void press();
	};
}