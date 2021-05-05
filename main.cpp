#include <blah.h>
#include "game.h"
#include <irrKlang.h>
#include <iostream>
#include "dear_imgui.h"

#include "imgui.h"

using namespace Blah;
using namespace BT;

namespace {
	Game game;

	void startup()
	{
		game.startup();
		DearImgui::startup();
	}

	void shutdown()
	{
		game.shutdown();
	}

	void update()
	{
		static bool pause = false;

		DearImgui::update();
		game.update();

		ImGui::EndFrame();
	}

	void render()
	{
		game.render();
		DearImgui::render();
	}
}

int main() {
	Config config;
	config.name = "Blah Thingy";
	config.width = 1280;
	config.height = 720;

	config.on_startup = startup;
	config.on_shutdown = shutdown;
	config.on_update = update;
	config.on_render = render;

	App::run(&config);
}