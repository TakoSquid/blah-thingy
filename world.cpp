#include "world.h"
#include <algorithm>

using namespace Blah;
using namespace BT;

uint8_t BT::Component::type() const
{
	return m_type;
}

String BT::Component::name() const
{
	return m_name;
}

bool BT::Component::cached() const
{
	return m_cached;
}

Entity* Component::entity()
{
	BLAH_ASSERT(m_entity != nullptr, "Component has no entity ! Currently not supported !");
	return m_entity;
}

const Entity* Component::entity() const
{
	return m_entity;
}

World* Component::world()
{
	return (m_entity ? m_entity->world() : nullptr);
}

const World* Component::world() const
{
	return (m_entity ? m_entity->world() : nullptr);
}

Component* Component::prev()
{
	return m_prev;
}

const Component* Component::prev() const
{
	return m_prev;
}

Component* Component::next()
{
	return m_next;
}

const Component* Component::next() const
{
	return m_next;
}

void Component::destroy()
{
	if (m_entity && m_entity->world())
	{
		m_entity->world()->destroy(this);
	}
}

void Component::awake() {}
void Component::update() {}
void Component::render(Blah::Batch& batch) {}
void Component::destroyed() {}

World* Entity::world()
{
	return m_world;
}

const World* Entity::world() const
{
	return m_world;
}

Entity* Entity::prev()
{
	return m_prev;
}

const Entity* Entity::prev() const
{
	return m_prev;
}

Entity* Entity::next()
{
	return m_next;
}

const Entity* Entity::next() const
{
	return m_next;
}

Blah::Vector<Component*>& Entity::components()
{
	return m_components;
}

void Entity::destroy()
{
	m_world->destroy_entity(this);
}

const Blah::Vector<Component*>& Entity::components() const
{
	return m_components;
}

World::~World()
{
	while (m_alive.first)
		destroy_entity(m_alive.first);

	for (int i = 0; i < Component::Types::count(); i++)
	{
		Component* c = m_components_cache[i].first;
		while (c)
		{
			Component* next = c->m_next;
			delete c;
			c = next;
		}
	}

	Entity* e = m_cache.first;
	while (e)
	{
		Entity* next = e->m_next;
		delete e;
		e = next;
	}
}

Entity* World::add_entity(Point point)
{
	Entity* instance;

	if (m_cache.first)
	{
		instance = m_cache.first;
		m_cache.remove(instance);
		*instance = Entity();
	}
	else
	{
		instance = new Entity();
	}

	m_alive.insert(instance);

	instance->position = point;
	instance->m_world = this;

	return instance;
}

Entity* World::first_entity()
{
	return m_alive.first;
}

const Entity* World::first_entity() const
{
	return m_alive.first;
}

Entity* World::last_entity()
{
	return m_alive.last;
}

const Entity* World::last_entity() const
{
	return m_alive.last;
}

void World::destroy_entity(Entity* entity)
{
	if (entity && entity->m_world == this)
	{
		for (int i = entity->m_components.size() - 1; i >= 0; i--)
			destroy(entity->m_components[i]);

		m_alive.remove(entity);
		m_cache.insert(entity);

		entity->m_world = nullptr;
	}
}

void World::destroy(Component* component)
{
	if (component && component->m_entity && component->m_entity->m_world == this)
	{
		auto type = component->m_type;

		component->destroyed();

		auto& list = component->m_entity->m_components;
		for (int i = list.size() - 1; i >= 0; i--)
		{
			if (list[i] == component)
			{
				list.erase(i);
				break;
			}
		}

		m_components_alive[type].remove(component);
		m_components_cache[type].insert(component);
		component->m_cached = true;
	}
}

void World::clear()
{
	Entity* entity = first_entity();
	while (entity)
	{
		auto next = entity->next();
		destroy_entity(entity);
		entity = next;
	}
}

void World::update()
{
	for (int i = 0; i < Component::Types::count(); i++)
	{
		auto component = m_components_alive[i].first;

		while (component)
		{
			auto next = component->m_next;
			if (component->active && component->m_entity->active)
				component->update();
			component = next;
		}
	}
}

void World::render(Blah::Batch& batch)
{
	// Meh

	for (int i = 0; i < Component::Types::count(); i++)
	{
		auto component = m_components_alive[i].first;
		while (component)
		{
			if (component->visible && component->m_entity->visible)
				m_visible.push_back(component);
			component = component->m_next;
		}
	}

	std::sort(m_visible.begin(), m_visible.end(), [](const Component* a, const Component* b)
		{
			return a->depth > b->depth;
		});

	for (auto& it : m_visible)
		it->render(batch);

	m_visible.clear();
}