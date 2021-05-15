#include "moving_platform.h"
#include "imgui.h"
#include "masks.h"

namespace BT
{
	void MovingPlatform::update()
	{
		static float progress = 0.0f;
		int sign = Calc::sign(velocity.x);

		movers.clear();

		for (const auto& col : collider->all(Mask::player, Point(0, -1)))
		{
			movers.insert(col->get<Mover>());
		}

		for (const auto& col : collider->all(Mask::player, Point(sign, 0)))
		{
			movers.insert(col->get<Mover>());
		}

		if (velocity != Point(0, 0))
		{
			//auto next = Calc::approach(entity()->position, entity()->position + Point(-1, 0), Time::delta);
			//entity()->position = Point(next.x, next.y);

			
			Vec2 total = remainer + velocity * Time::delta;
			Point to_move = Point((int)total.x, (int)total.y);
			remainer.x = total.x - to_move.x;
			remainer.y = total.y - to_move.y;


			entity()->position += to_move;

			for (auto mover : movers)
			{
				mover->move_y(to_move.y);
				mover->move_x(to_move.x);
			}
		}

	}

	void MovingPlatform::debug()
	{
		ImGui::Text("Size : %d", movers.size());

		if (ImGui::BeginListBox("Move objets"))
		{
			for (const auto& mover : movers)
				ImGui::Text("%s", mover->entity()->name.cstr());
			
			ImGui::EndListBox();
		}
	}
}