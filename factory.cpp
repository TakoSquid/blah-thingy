#include "factory.h"

#include "talkative.h"
#include "animator.h"
#include "collider.h"
#include "masks.h"
#include "mover.h"
#include "player.h"
#include "debugSolid.h"
#include "mech.h"
#include "button.h"
#include "signal_box.h"

using namespace BT;

Entity* Factory::player(World* world, Point point) {
	auto en = world->add_entity(point);
	en->name = "player";

	auto anim = en->add(Animator("player"));
	anim->play("idle");
	anim->depth = -10;

	auto hitbox = en->add(Collider::make_rect(RectI(-4, -12, 8, 12)));
	hitbox->mask = Mask::player;

	auto slope_collider = en->add(Collider::make_rect(RectI(-1, -2, 2, 2)));
	slope_collider->mask = Mask::player;

	auto mover = en->add(Mover());
	mover->collider = hitbox;
	mover->slope_collider = slope_collider;

	en->add(Player());

	return en;
}

Entity* BT::Factory::debugSolid(World* world, Point pos, Point size, uint32_t mask)
{
	auto en = world->add_entity(Point(pos));
	en->name = "debug solid";

	auto hitbox = en->add(Collider::make_rect(RectI(pos.x, pos.y, size.x, size.y)));
	hitbox->mask = mask;

	en->add(DebugSolid());

	return en;
}

Entity* BT::Factory::mech(World* world, Point pos)
{
	auto en = world->add_entity(pos);
	en->name = "mech";

	auto animator = en->add(Animator("mech"));
	animator->play("idle");
	animator->depth = -9;

	auto hitbox = en->add(Collider::make_rect(RectI(-10, -77, 20, 77)));
	hitbox->mask = Mask::player;

	auto slope_collider = en->add(Collider::make_rect(RectI(-1, -2, 2, 2)));
	slope_collider->mask = Mask::player;

	auto mover = en->add(Mover());
	mover->gravity = 450;
	mover->collider = hitbox;
	mover->slope_collider = slope_collider;

	auto player = en->add(Player());
	player->active = false;
	player->max_ground_speed = 150;
	player->max_air_speed = 120;

	auto get_in_sprite = en->add(Animator("btn_y"));
	get_in_sprite->depth = -10;
	get_in_sprite->offset = Point(-4, -30 - 79);
	get_in_sprite->visible = false;

	auto mech = en->add(Mech());
	mech->depth = -8;
	mech->pick_up_indicator = get_in_sprite;

	auto pick_up_zone = en->add(Collider::make_rect(RectI(-20, -79, 40, 1)));
	mech->pick_up_zone = pick_up_zone;

	auto platform = en->add(Collider::make_rect(RectI(-20, -78, 40, 1)));
	platform->mask = Mask::jumpthru;

	return en;
}

Entity* BT::Factory::button(World* world, Point point)
{ 
	auto en = world->add_entity(point);
	en->name = "button";

	auto button = en->add(Button());

	auto animator = en->add(Animator("button"));
	animator->play("idle");
	animator->depth = -1;

	auto trigger_zone = en->add(Collider::make_rect(RectI(-8, -8, 16, 8)));
	button->trigger_zone = trigger_zone;

	auto signal_box = en->add(SignalBox());
	button->signal_box = signal_box;

	return en;
}