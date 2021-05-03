#include "sprite.h"

using namespace BT;

const Sprite::Animation* BT::Sprite::get_animation(const String& name) const
{
	for (auto& it : animations)
		if (it.name == name)
			return &it;

	return nullptr;
}