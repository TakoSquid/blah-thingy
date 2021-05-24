#include "button.h"
#include "masks.h"
#include "animator.h"
#include "signal_box.h"
#include "content.h"
#include "mover.h"

BT::Button::Button(bool pressed)
	:m_pressed(pressed),
	m_pressed_old(pressed),
	trigger_zone(nullptr)
{
}

void BT::Button::update()
{
	Collider* other_col = trigger_zone->first(Mask::player);

	if (other_col && Calc::sign(other_col->get<Mover>()->speed.y) == 1)
		m_pressed = true;

	if (!m_pressed_old && m_pressed) {
		Content::play_sound("button");
		signal_box->activate();
	}

	if(auto animator = entity()->get<Animator>())
		m_pressed ? animator->play("pressed") : animator->play("idle");

	m_pressed_old = m_pressed;
}