#include "button.h"
#include "masks.h"
#include "animator.h"

BT::Button::Button(bool pressed)
	:m_pressed(pressed),
	m_pressed_old(pressed),
	trigger_zone(nullptr),
	on_press([](Button* self) {Log::info("button pressed"); })
{
}

void BT::Button::press()
{
	on_press(this);
}

void BT::Button::update()
{
	if (!trigger_zone->cached() && trigger_zone->check(Mask::player))
		m_pressed = true;

	if (!m_pressed_old && m_pressed)
		press();

	if(auto animator = entity()->get<Animator>())
		m_pressed ? animator->play("pressed") : animator->play("idle");

	m_pressed_old = m_pressed;
}