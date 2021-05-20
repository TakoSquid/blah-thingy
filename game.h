#pragma once

#include "camera.h"
#include <blah.h>
#include "world.h"
#include "factory.h"
#include <irrKlang.h>
#include "content.h"

using namespace Blah;
using namespace irrklang;

namespace BT
{
	class Game
	{
	public:
		static constexpr int width = 256;
		static constexpr int height = 224;
		static constexpr int tile_width = 16;
		static constexpr int tile_height = 16;
		static constexpr int columns = width / tile_width;
		static constexpr int rows = height / tile_height;

		World world;
		FrameBufferRef buffer;
		Batch batch;
		Camera camera;
		const RoomInfo* room;

		/*void load_room(Point cell, bool is_reload = false);*/
		void load_room(String name, bool is_reload = false);

		void startup();
		void shutdown();
		void update();
		void render();

	private:
		bool m_draw_pivot;
		bool m_draw_colliders;

		bool m_draw_solid;
		bool m_draw_jumpthrough;
		bool m_draw_slope;

		bool m_draw_sb;

		Vector<Entity*> m_last_entities;

		Point m_next_room;
		Point m_last_room;
	};
}