#pragma once
#include <blah.h>

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
	};
}