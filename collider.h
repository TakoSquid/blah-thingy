#pragma once
#include <blah.h>
#include "world.h"

using namespace Blah;

namespace BT
{
	class Collider : public Component
	{
	public:
		enum class Shape
		{
			None,
			Rect,
			Grid,
			Slope,
		};

		struct Slope
		{
			RectI AABB;
			int facing;

			float get_angle()
			{
				return Calc::atan2(AABB.h, AABB.w);
			};

			float get_speed_mult(bool climbing = true)
			{
				float value = 1 - (get_angle() / (Calc::PI / 2));

				return climbing ? value : 1 / value;
			};
		};

		uint32_t mask = 0;

		Collider();

		static Collider make_rect(const RectI& rect);
		static Collider make_grid(int tile_size, int columns, int rows);
		static Collider make_slope(const RectI& rect, int facing);

		Shape shape() const;
		RectI get_rect() const;
		void set_rect(const RectI& value);
		bool get_cell(int x, int y) const;
		void set_cell(int x, int y, bool value);
		void set_cells(int x, int y, int w, int h, bool value);
		Slope get_slope() const;

		Collider* first(uint32_t mask, Point offset = Point::zero);
		const Collider* first(uint32_t mask, Point offset = Point::zero) const;

		Vector<Collider*> all(uint32_t mask, Point offset = Point::zero);

		bool check(uint32_t mask, Point offset = Point::zero) const;
		bool overlaps(const Collider* other, Point offset = Point::zero) const;

		void render(Batch& batch) override;
		void debug() override;

	private:
		struct Grid
		{
			int columns = -1;
			int rows = -1;
			int tile_size = -1;
			Vector<bool> cells;
		};

		Shape m_shape = Shape::None;
		RectI m_rect;
		Grid m_grid;
		Slope m_slope;

		static bool rect_to_rect(const Collider* a, const Collider* b, Point offset);
		static bool rect_to_grid(const Collider* a, const Collider* b, Point offset);
		static bool rect_to_slope(const Collider* a, const Collider* b, Point offset);
	};
}