#include "camera.h"
#include <iostream>

#include "imgui.h"

using namespace BT;

Camera::Camera(Rect posSize, float rot, Vec2 scale)
	:posSize(posSize),
	rotation(rot),
	scale(scale),
	in_transition(false),
	transition_duration(.75f),
	transition_timer(0.0f),
	old_position(Vec2(0, 0)),
	using_allowed_area(false)
{
}

Mat3x2 Camera::get_matrix()
{
	return Mat3x2::create_translation(-posSize.top_left()) * Mat3x2::create_rotation(rotation) * Mat3x2::create_scale(scale);
	
}

void Camera::set_allowed_area(Rect area)
{
	allowed_area = area;
}

Rect BT::Camera::get_allowed_area()
{
	return allowed_area;
}

void Camera::set_position(Vec2 pos)
{
	posSize.x = pos.x;
	posSize.y = pos.y;
}

Vec2 Camera::get_position() const
{
	return Vec2(posSize.x, posSize.y);
}

void BT::Camera::set_scale(Vec2 scale)
{
	this->scale = scale;
}

void BT::Camera::set_size(Vec2 size)
{
	posSize.w = size.x;
	posSize.h = size.y;
}

Vec2 BT::Camera::get_size()
{
	return Vec2(posSize.w, posSize.h);
}

void Camera::lerp_to(Vec2 target_pos, Vec2 s)
{
	in_transition = true;
	transition_timer = 0.0f;

	old_position = posSize.top_left();
	target_position = target_pos;

	old_scale = scale;
	target_scale = s;

	Log::info("target scale = %f %f", target_scale.x, target_scale.y);
}

bool Camera::is_in_transition()
{
	return in_transition;
}

void Camera::update()
{
	if (in_transition)
	{
		transition_timer += Time::delta;

		auto t = transition_timer / transition_duration;

		auto new_pos = old_position + (target_position - old_position) * Ease::quad_in_out(t);
		posSize.x = new_pos.x;
		posSize.y = new_pos.y;

		auto new_scale = old_scale + (target_scale - old_scale) * ((old_scale.length_squared() > target_scale.length_squared()) ? Ease::quad_out(t) : Ease::quad_in(t));
		scale = new_scale;

		if (t >= 1.0f)
		{
			in_transition = false;
			transition_timer = 0.0f;

			posSize.x = target_position.x;
			posSize.y = target_position.y;
			scale = target_scale;
		}
	}
	else if (using_allowed_area)
	{
		posSize.x = Calc::clamp(posSize.right() - posSize.w, allowed_area.left(), allowed_area.right() - posSize.w);
		posSize.y = Calc::clamp(posSize.bottom() - posSize.h, allowed_area.top(), allowed_area.bottom() - posSize.h);
	}

	if (ImGui::Begin("Camera", 0, ImGuiWindowFlags_AlwaysAutoResize))
	{
		float scale_array []= { scale.x, scale.y };
		ImGui::DragFloat2("scale", scale_array, .01f, 0.0f, 0.0f, "%.3f", 0);
		scale = Vec2(scale_array[0], scale_array[1]);

		ImGui::End();
	}
}