#pragma once

#include <blah.h>
#include "content.h"

namespace BT {
	namespace DearImgui
	{
		void startup();
		void update();
		void render();

		void display_texture(TextureRef texture, const Vec2 size, const Vec2 uv0 = Vec2::zero, const Vec2 uv1 = Vec2::one, const Vec4& tint_col = Vec4(1, 1, 1, 1), const Vec4& border_col = Vec4(0, 0, 0, 0));

		extern TextureRef game;
	}
}