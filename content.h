#pragma once
#include <blah.h>
#include "json.h"
#include <irrKlang.h>

using namespace Blah;
using namespace nlohmann;
using namespace irrklang;

namespace BT {
	struct Sprite;
	struct Tileset;

	struct TileLayer {
		String name;
		String tileset;
		int row;
		int column;

		Vector<int> data; // TODO : Do an array for tile data
	};

	struct ParsedEntity {
		Point position;
		String name;
		int id;
		json values;
	};

	struct RoomInfo
	{
		//Point cell;
		Point offset;
		Point size;
		String name;

		Vector<TileLayer> layers;
		Vector<ParsedEntity> entities;
	};

	struct SoundInfo
	{
		String name;
		ISoundSource* sound_source;
	};

	class Content {
	public:
		static SpriteFont font;

		static irrklang::ISoundEngine* sound_engine;

		static FilePath path();
		static void load();
		static void unload();
		static TextureRef atlas();

		static const Sprite* find_sprite(const char* name);
		static const Tileset* find_tileset(const char* name);
		/*static const RoomInfo* find_room(const Point& cell);*/
		static const RoomInfo* find_room(const String& name);
		static const RoomInfo* find_room_by_pos(const Point& position);
		static void play_sound(const char* name, bool playLooped = false, float playbackSpeed = 1.0f);
	private:
		static Vector<SoundInfo*> sounds;
	};
}