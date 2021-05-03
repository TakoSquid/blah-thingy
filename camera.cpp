#include "camera.h"
#include <iostream>

BT::Camera::Camera(Vec2 pos, float rot, Point size, Vec2 scale)
	:position(pos),
	rotation(rot),
	size(size), scale(scale),
	in_transition(false),
	transition_duration(.75f),
	transition_timer(0.0f),
	old_position(Vec2(0, 0))
{
}

Mat3x2 BT::Camera::get_matrix()
{
	return Mat3x2::create_translation(-Vec2(position.x, position.y)) * Mat3x2::create_rotation(rotation) * Mat3x2::create_scale(scale);
}

void BT::Camera::set_position(Vec2 pos)
{
	position = pos;
}

Vec2 BT::Camera::get_position() const
{
	return position;
}

void BT::Camera::lerp_to(Vec2 pos)
{
	in_transition = true;
	transition_timer = 0.0f;
	old_position = position;
	target_position = pos;
}

bool BT::Camera::is_in_transition()
{
	return in_transition;
}

void BT::Camera::update()
{
	if (!in_transition)
		return;

	transition_timer += Time::delta;

	position = old_position + (target_position - old_position) * Ease::quad_in_out(transition_timer / transition_duration);

	if (transition_timer >= transition_duration)
	{
		in_transition = false;
		transition_timer = 0.0f;
	}
}