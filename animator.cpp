#include "animator.h"
#include "content.h"
#include <iostream>
#include "imgui.h"
#include <sstream>

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

void Animator::debug()
{
	std::stringstream ss;

	ss << "Animation##" << this;
	ImGui::Text(ss.str().c_str());

	if (ImGui::BeginCombo(ss.str().c_str(), debug_selected_animation.name))
	{
		ss.str("");
		for (const auto& anim : m_sprite->animations)
		{
			bool selected = (debug_selected_animation.name == anim.name);

			ss << anim.name << "##" << this;
			if (ImGui::Selectable(ss.str().c_str(), selected))
				debug_selected_animation = anim;
			ss.str("");

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}
	ss.str("");
	ImGui::SameLine();
	
	ss << "restart##" << this;
	ImGui::Checkbox(ss.str().c_str(), &debug_restart);
	ss.str("");

	ImGui::SameLine();

	ss << "play once##" << this;
	ImGui::Checkbox(ss.str().c_str(), &debug_play_once);
	ss.str("");

	ImGui::SameLine();

	ss << "Play !##" << this;
	if (ImGui::Button(ss.str().c_str()))
	{
		play(debug_selected_animation.name, debug_restart, debug_play_once);
	}

	ss.str("");

	ss << "scale##" << this;
	float scale[2] = { this->scale.x , this->scale.y };
	ImGui::InputFloat2(ss.str().c_str(), scale);
	this->scale = Vec2(scale[0], scale[1]);
	ss.str("");

	ss << "offset##" << this;
	int offset[2] = { this->offset.x, this->offset.y };
	ImGui::InputInt2(ss.str().c_str(), offset);
	this->offset = Point(offset[0], offset[1]);
	ss.str("");

	ss << "in valid state : " << std::boolalpha << in_valid_state();
	ImGui::Text(ss.str().c_str());
	ss.str("");

	ss << "play once : " << std::boolalpha << m_play_once;
	ImGui::Text(ss.str().c_str());
	ss.str("");

	ss << "currently playing : " << m_sprite->animations[m_animation_index].name << std::endl;
	ImGui::Text(ss.str().c_str());
	ss.str("");

}