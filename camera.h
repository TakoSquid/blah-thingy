#pragma once
#include <blah.h>
#include "world.h"

using namespace Blah;

namespace BT
{
	// Confusing camera size and scale
	// posSize.w and posSize.h used as hitbox in the allowed area
	// zoom is handled by the scale vector

	class Camera
	{
	public:
		Camera(Rect posSize = Rect(0, 0, 256, 224), float rot = 0.0f, Vec2 scale = Vec2::one);

		void update();

		Mat3x2 get_matrix();
		void set_position(Vec2 pos);
		Vec2 get_position() const;
		void set_scale(Vec2 scale);
		void set_size(Vec2 size);
		Vec2 get_size();

		void lerp_to(Vec2 pos, Vec2 target_scale = Vec2::one);

		bool is_in_transition();

		void set_allowed_area(Rect area);
		Rect get_allowed_area();
		bool using_allowed_area;

		void set_target_box(Rect area);
		Rect get_target_box();
		void update_rect_target(Rect target);
		bool using_target_box;

		void shake(float frequency = 2.0f,float amplitude = 5.0f, float shake_duration = .3f);

	private:
		float rotation;
		Vec2 scale;

		Rect posSize;

		bool in_transition;
		float transition_duration;
		float transition_timer;

		Vec2 old_position;
		Vec2 target_position;

		Vec2 old_scale;
		Vec2 target_scale;

		Rect allowed_area;
		Rect target_box;

		// Shake data
		Vec2 shake_offset;

		float m_frequency = 2.0f;
		float m_amplitude = 5.0;
		float m_shake_duration = .3f;
		float m_shake_timer = 0.0f;
	};
}