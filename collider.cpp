#include "collider.h"

#include "masks.h"
#include <iostream>

using namespace BT;

Collider::Collider()
{
	visible = false;
	active = true;
}

Collider Collider::make_rect(const RectI& rect)
{
	Collider collider;
	collider.m_shape = Shape::Rect;
	collider.m_rect = rect;
	return collider;
}

Collider Collider::make_grid(int tile_size, int columns, int rows)
{
	Collider collider;
	collider.m_shape = Shape::Grid;
	collider.m_grid.tile_size = tile_size;
	collider.m_grid.columns = columns;
	collider.m_grid.rows = rows;
	collider.m_grid.cells.expand(columns * rows);
	return collider;
}

Collider Collider::make_slope(const RectI& rect, int facing)
{
	Collider collider;
	collider.m_shape = Shape::Slope;
	collider.m_slope.AABB = rect;
	collider.m_slope.facing = facing;
	return collider;
}

Collider::Shape Collider::shape() const
{
	return m_shape;
}

RectI Collider::get_rect() const
{
	BLAH_ASSERT(m_shape == Shape::Rect, "Collider is not a Rectangle");
	return m_rect;
}

void Collider::set_rect(const RectI& value)
{
	BLAH_ASSERT(m_shape == Shape::Rect, "Collider is not a Rectangle");
	m_rect = value;
}

bool Collider::get_cell(int x, int y) const
{
	BLAH_ASSERT(m_shape == Shape::Grid, "Collider is not a Grid");
	BLAH_ASSERT(x >= 0 && y >= 0 && x < m_grid.columns&& y < m_grid.rows, "Cell is out of bounds");

	return m_grid.cells[x + y * m_grid.columns];
}

void Collider::set_cell(int x, int y, bool value)
{
	BLAH_ASSERT(m_shape == Shape::Grid, "Collider is not a Grid");
	BLAH_ASSERT(x >= 0 && y >= 0 && x < m_grid.columns&& y < m_grid.rows, "Cell is out of bounds");

	m_grid.cells[x + y * m_grid.columns] = value;
}

void Collider::set_cells(int x, int y, int w, int h, bool value)
{
	for (int tx = x; tx < x + w; tx++)
		for (int ty = y; ty < y + h; ty++)
			m_grid.cells[tx + ty * m_grid.columns] = value;
}

Collider::Slope Collider::get_slope() const
{
	BLAH_ASSERT(m_shape == Shape::Slope, "Collider is not a slope");

	return m_slope;
}

bool Collider::check(uint32_t mask, Point offset) const
{
	return first(mask, offset) != nullptr;
}

Collider* Collider::first(uint32_t mask, Point offset)
{
	if (world())
	{
		auto other = world()->first<Collider>();

		while (other)
		{
			if (other != this &&
				other->active &&
				(other->mask & mask) != 0 &&
				overlaps(other, offset))
				return other;

			other = (Collider*)other->next();
		}
	}

	return nullptr;
}

const Collider* Collider::first(uint32_t mask, Point offset) const
{
	if (world())
	{
		auto other = world()->first<Collider>();
		while (other)
		{
			if (other != this &&
				other->active &&
				(other->mask & mask) == mask &&
				overlaps(other, offset))
				return other;

			other = (Collider*)other->next();
		}
	}

	return nullptr;
}

Vector<Collider*> Collider::all(uint32_t mask, Point offset)
{
	Vector<Collider*> found;

	if (world())
	{
		auto other = world()->first<Collider>();
		while (other)
		{
			if (other != this &&
				(other->mask & mask) == mask &&
				overlaps(other, offset))
				found.emplace_back(other);

			other = (Collider*)other->next();
		}
	}

	return found;
}

bool Collider::overlaps(const Collider* other, Point offset) const
{
	BLAH_ASSERT(other != nullptr, "Other collider is missing !");

	if (m_shape == Shape::Rect)
	{
		if (other->m_shape == Shape::Rect)
		{
			return rect_to_rect(this, other, offset);
		}
		else if (other->m_shape == Shape::Grid)
		{
			return rect_to_grid(this, other, offset);
		}
		else if (other->m_shape == Shape::Slope)
		{
			return rect_to_slope(this, other, offset);
		}
	}
	else if (m_shape == Shape::Grid)
	{
		if (other->m_shape == Shape::Rect)
		{
			return rect_to_grid(other, this, -offset);
		}
		else if (other->m_shape == Shape::Grid)
		{
			BLAH_ASSERT(false, "Grid->Grid Overlap checks not supported!");
		}
		else if (other->m_shape == Shape::Slope)
		{
			BLAH_ASSERT(false, "Grid->Slope Overlap checks not supported!");
		}
	}

	return false;
}

void Collider::render(Batch& batch)
{
	Color color = Color::red;

	if (mask == Mask::jumpthru)
		color = Color::blue;

	if (mask == Mask::slope) {
		color = Color::yellow;
	}

	batch.push_matrix(Mat3x2::create_translation(entity()->position));

	if (m_shape == Shape::Rect)
	{
		RectI ouais = m_rect;

		batch.rect_line(ouais, 1, color);
	}
	else if (m_shape == Shape::Grid)
	{
		for (int x = 0; x < m_grid.columns; x++)
		{
			for (int y = 0; y < m_grid.rows; y++)
			{
				if (!m_grid.cells[x + y * m_grid.columns])
					continue;

				batch.rect_line(
					Rect((float)(x * m_grid.tile_size), (float)(y * m_grid.tile_size), (float)m_grid.tile_size, (float)m_grid.tile_size),
					1, color);
			}
		}
	}
	else if (m_shape == Shape::Slope)
	{
		if (m_slope.facing == 1)
		{
			batch.line(m_slope.AABB.top_left(), m_slope.AABB.bottom_right(), 1, color);
		}
		else if (m_slope.facing == -1)
		{
			batch.line(m_slope.AABB.top_right(), m_slope.AABB.bottom_left(), 1, color);
		}
	}

	batch.pop_matrix();
}

bool BT::Collider::rect_to_rect(const Collider* a, const Collider* b, Point offset)
{
	RectI ar = a->m_rect + a->entity()->position + offset;
	RectI br = b->m_rect + b->entity()->position;

	return ar.overlaps(br);
}

bool BT::Collider::rect_to_grid(const Collider* a, const Collider* b, Point offset)
{
	BLAH_ASSERT(a != nullptr, "Collider A is missing !");
	BLAH_ASSERT(b != nullptr, "Collider B is missing !");

	BLAH_ASSERT(a->entity() != nullptr, "Collider A does not have an entity !");


	// get a relative rectangle to the grid
	RectI rect = a->m_rect + a->entity()->position + offset - b->entity()->position;

	// get the cells the rectangle overlaps
	int left = Calc::clamp((int)Calc::floor(rect.x / (float)b->m_grid.tile_size), 0, b->m_grid.columns);
	int right = Calc::clamp((int)Calc::ceiling(rect.right() / (float)b->m_grid.tile_size), 0, b->m_grid.columns);
	int top = Calc::clamp((int)Calc::floor(rect.y / (float)b->m_grid.tile_size), 0, b->m_grid.rows);
	int bottom = Calc::clamp((int)Calc::ceiling(rect.bottom() / (float)b->m_grid.tile_size), 0, b->m_grid.rows);

	// check each cell
	for (int x = left; x < right; x++)
		for (int y = top; y < bottom; y++)
			if (b->m_grid.cells[x + y * b->m_grid.columns])
				return true;

	// all cells were empty
	return false;
}

bool BT::Collider::rect_to_slope(const Collider* a, const Collider* b, Point offset)
{
	RectI ari = a->m_rect + a->entity()->position + offset;
	RectI bri = b->m_slope.AABB + b->entity()->position;

	bool aabb_overlaps = ari.overlaps(bri);

	if (aabb_overlaps)
	{
		Rect ar(ari);

		if (b->m_slope.facing == 1)
		{
			return ar.intersects(bri.top_left(), bri.bottom_right());
		}
		else if (b->m_slope.facing == -1)
		{
			return ar.intersects(bri.bottom_left(), bri.top_right());
		}
	}

	return false;
}