#include "mover.h"
#include "masks.h"

#include <iostream>

using namespace BT;

bool Mover::move_x(int amount)
{

	if (collider)
	{
		int sign = Calc::sign(amount);

		while (amount != 0)
		{
			Collider* slope;

			if ((slope = slope_collider->first(Mask::slope, Point(0, 0))) && slope->get_slope().facing != sign)
			{
				int i = 1;

				// This value is to assure we exit the loop at some point
				while (i < 10)
				{
					if (!slope_collider->check(Mask::slope, Point(0, -i)))
					{
						entity()->position.y -= i;
						break;
					}
					i++;
				}
			}
			else if ((slope = slope_collider->first(Mask::slope, Point(0, 1))) && !slope_collider->first(Mask::slope, Point(sign, 1)) && slope->get_slope().facing == sign)
			{
				int i = 1;

				// This value is to assure we exit the loop at some point
				while (i < 10)
				{
					if (slope_collider->check(Mask::slope, Point(sign, i)) || slope_collider->check(Mask::solid, Point(sign, i)) || slope_collider->check(Mask::jumpthru, Point(sign, i)))
					{
						entity()->position.y += (i - 1);
						break;
					}
					i++;
				}
			}
			else if (collider->check(Mask::solid, Point(sign, 0)))
			{
				if (on_hit_x)
					on_hit_x(this);
				else
					stop_x();

				return true;
			}

			amount -= sign;
			entity()->position.x += sign;
		}
	}
	else {
		entity()->position.x += amount;
	}
	return false;
}

bool Mover::move_y(int amount)
{
	if (collider)
	{
		int sign = Calc::sign(amount);

		while (amount != 0)
		{
			bool hit_something = collider->check(Mask::solid, Point(0, sign));

			// Not touched anything solid and moving down

			if (!hit_something && sign > 0)
			{
				hit_something = slope_collider->check(Mask::slope, Point(0, sign));
			}

			if (!hit_something && sign > 0) {
				hit_something = collider->check(Mask::jumpthru, Point(0, sign)) && !collider->check(Mask::jumpthru, Point(0, 0));
			}

			if (hit_something)
			{
				if (on_hit_y)
					on_hit_y(this);
				else
					stop_y();
				return true;
			}

			amount -= sign;
			entity()->position.y += sign;
		}
	}
	else
	{
		entity()->position.y += amount;
	}
	return false;
}

void Mover::stop_x()
{
	speed.x = 0;
	m_remainer.x = 0;
}

void Mover::stop_y()
{
	speed.y = 0;
	m_remainer.y = 0;
}

void Mover::stop()
{
	speed.x = 0;
	speed.y = 0;
	m_remainer.x = 0;
	m_remainer.y = 0;
}

bool Mover::on_ground(int dist) const
{

	if (!collider)
		return false;

	return
		collider->check(Mask::solid, Point(0, dist))
		|| (collider->check(Mask::jumpthru, Point(0, dist)) && !collider->check(Mask::jumpthru, Point(0, 0))
			|| slope_collider->check(Mask::slope, Point(0, dist)));
}

Collider* Mover::on_slope(int dist) const
{
	if (!collider)
		return nullptr;

	return slope_collider->first(Mask::slope, Point(0, dist));
}

void Mover::update()
{
	if (friction > 0 && on_ground())
		speed.x = Calc::approach(speed.x, 0, friction * Time::delta);

	if (gravity != 0 && (!collider || !collider->check(Mask::solid, Point(0, 1))))
		speed.y += gravity * Time::delta;

	Vec2 total = m_remainer + speed * Time::delta;

	Point to_move = Point((int)total.x, (int)total.y);

	m_remainer.x = total.x - to_move.x;
	m_remainer.y = total.y - to_move.y;

	move_x(to_move.x);
	move_y(to_move.y);
}