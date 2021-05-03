#pragma once
#include <blah.h>
#include <functional>
#include "world.h"
#include "collider.h"

using namespace Blah;

namespace BT
{
	class Button : public Component
	{
	public:
		Button(bool pressed = false);

		std::function<void(Button* self)> on_press;

		void update() override;

		Collider* trigger_zone;
	private:
		bool m_pressed;
		bool m_pressed_old;
		void press();
	};
}