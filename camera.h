#pragma once
#include <blah.h>

using namespace Blah;

namespace BT
{
	class Camera
	{
	public:
		Camera(Vec2 pos = Vec2::zero, float rot = 0.0f, Point size = Point(256, 160), Vec2 scale = Vec2::one);

		void update();

		Mat3x2 get_matrix();
		void set_position(Vec2 pos);
		Vec2 get_position() const;
		void lerp_to(Vec2 pos);
		bool is_in_transition();

	private:
		Vec2 position;
		float rotation;
		Point size;
		Vec2 scale;

		bool in_transition;
		float transition_duration;
		float transition_timer;
		Vec2 old_position;
		Vec2 target_position;
	};
}