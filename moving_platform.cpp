#include "moving_platform.h"
#include "imgui.h"
#include "masks.h"

namespace BT
{
	void MovingPlatform::update()
	{
		auto velocity = calculate_platform_movement();
		velocity_to_display = velocity;

		calculate_passenger_movement(velocity);
		move_passengers(true);
		entity()->position += velocity;
		move_passengers(false);
	}

	void MovingPlatform::debug()
	{
		ImGui::Text("Movements size : %d", passengers.size());
		ImGui::Text("Velocity : {%d : %d}", velocity_to_display.x, velocity_to_display.y);
		ImGui::DragFloat("speed", &speed, 10, 0, 500);

		ImGui::PushItemWidth(150);
		if (ImGui::BeginListBox("Movements"))
		{
			for (const auto& passenger : passengers)
				ImGui::Text("%s : {%d : %d}", passenger.entity->name.cstr(), passenger.movement.x, passenger.movement.y);

			ImGui::EndListBox();
		}
		ImGui::PopItemWidth();
	}

	Point MovingPlatform::calculate_platform_movement()
	{
		if (speed == 0.0f)
			return Point::zero;

		auto start = checkpoints[index];
		auto end = checkpoints[(index + 1) % checkpoints.size()];

		float length = (end - start).length();
		t += Time::delta * speed / length;
		t = Calc::clamp(t, 0.0f, 1.0f);

		auto next_pos = Vec2::lerp(start, end, t);

		auto velocity = next_pos - entity()->position;

		Vec2 total = remainer + velocity;
		Point to_move = Point((int)total.x, (int)total.y);
		remainer.x = total.x - to_move.x;
		remainer.y = total.y - to_move.y;

		if (t >= 1.0f)
		{
			t = 0.0f;
			index++;
			index %= checkpoints.size();
		}

		return to_move;
	}

	void MovingPlatform::calculate_passenger_movement(Point velocity)
	{
		passengers.clear();

		int directionY = Calc::sign(velocity.y);
		int directionX = Calc::sign(velocity.x);

		if (velocity.y != 0)
		{
			int length = Calc::abs(velocity.y);
			std::unordered_map<Entity*, Passenger> found;

			for (size_t i = 0; i < length; i++)
			{
				for (const auto& col : collider->all(Mask::player, Point(0, length* directionY)))
				{
					auto pushY = (length - i) * directionY;
					auto pushX = (directionY == -1) ? velocity.x : 0;

					if (found.find(col->entity()) == found.end())
					{
						found[col->entity()] = Passenger(col->entity(), Point(pushX, pushY), directionY == -1, true);
					}
				}
			}

			for (const auto& it : found)
			{
				passengers.emplace_back(it.second);
			}
		}

		if (velocity.x != 0)
		{
			int length = Calc::abs(velocity.x);
			std::unordered_map<Entity*, Passenger> found;

			for (size_t i = 0; i < length; i++)
			{
				for (const auto& col : collider->all(Mask::player, Point(length* directionX, 0)))
				{
					auto pushX = (length - i) * directionX;
					auto pushY = 0;

					if (found.find(col->entity()) == found.end())
					{
						found[col->entity()] = Passenger(col->entity(), Point(pushX, pushY), false, true);
					}
				}
			}

			for (const auto& it : found)
			{
				passengers.emplace_back(it.second);
			}
		}

		if (directionY == 1 || (velocity.y == 0 && velocity.x != 0))
		{
			std::unordered_map<Entity*, Passenger> found;

			for (const auto& col : collider->all(Mask::player, Point(0, -1)))
			{
				auto pushX = velocity.x;
				auto pushY = velocity.y;

				if (found.find(col->entity()) == found.end())
				{
					found[col->entity()] = Passenger(col->entity(), Point(pushX, pushY), true, false);
				}
			}

			for (const auto& it : found)
			{
				passengers.emplace_back(it.second);
			}
		}
	}

	void MovingPlatform::move_passengers(bool beforePlatform)
	{
		for (auto& passenger : passengers)
		{
			if (passenger.move_before_platform == beforePlatform)
			{
				Mover* mover = passenger.entity->get<Mover>();
				if (mover)
				{
					mover->move_x(passenger.movement.x);
					mover->move_y(passenger.movement.y);
				}
			}
		}
	}
}