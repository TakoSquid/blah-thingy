#include "animator.h"
#include "content.h"
#include <iostream>

using namespace BT;

Animator::Animator(const String& sprite)
{
	m_sprite = Content::find_sprite(sprite);
	m_animation_index = 0;
}

const Sprite* Animator::sprite() const
{
	return m_sprite;
}

const Sprite::Animation* Animator::animation() const
{
	if (m_sprite && m_animation_index >= 0 && m_animation_index < m_sprite->animations.size())
		return &m_sprite->animations[m_animation_index];

	return nullptr;
}

void Animator::play(const String& animation, bool restart, bool play_once)
{
	BLAH_ASSERT(m_sprite, "No sprite assigned :(");

	m_play_once = play_once;

	for (int i = 0; i < m_sprite->animations.size(); i++)
	{
		if (m_sprite->animations[i].name == animation)
		{
			if (m_animation_index != i || restart)
			{
				m_animation_index = i;
				m_frame_index = 0;
				m_frame_counter = 0;
			}

			break;
		}
	}
}

void Animator::update()
{
	if (in_valid_state())
	{
		auto& anim = m_sprite->animations[m_animation_index];
		auto& frame = anim.frames[m_frame_index];

		m_frame_counter += Time::delta;

		while (m_frame_counter >= frame.duration)
		{
			m_frame_counter -= frame.duration;
			m_frame_index++;
			if (m_frame_index >= anim.frames.size())
				m_frame_index = m_play_once ? anim.frames.size() - 1 : m_frame_index = 0;
		}
	}
}

void Animator::render(Batch& batch)
{
	if (in_valid_state())
	{
		Blah::Vec2 total_scale(scale.x * entity()->scale.x, scale.y * entity()->scale.y);

		batch.push_matrix(
			Mat3x2::create_transform(entity()->position + offset, m_sprite->origin, total_scale, entity()->rotation)
		);

		auto& anim = m_sprite->animations[m_animation_index];
		auto& frame = anim.frames[m_frame_index];
		batch.tex(frame.image, Vec2::zero, Color::white);

		batch.pop_matrix();
	}
}

bool Animator::in_valid_state() const
{
	return
		m_sprite &&
		m_animation_index >= 0 &&
		m_animation_index < m_sprite->animations.size() &&
		m_frame_index >= 0 &&
		m_frame_index < m_sprite->animations[m_animation_index].frames.size();
}