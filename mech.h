#pragma once
#include "world.h"
#include <blah.h>
#include "player.h"

using namespace Blah;

namespace BT
{
	class Mech : public Component
	{
	public:
		Mech();

		void awake() override;
		void render(Blah::Batch& batch) override;
		void update() override;

		Collider* pick_up_zone = nullptr;
		Component* pick_up_indicator = nullptr;

	private:
		Entity* m_pilot = nullptr;
		Player* m_mech_control = nullptr;

		Binding input_get_in;

		void set_pilot(Entity* pilot);
		void release_pilot();
	};
}