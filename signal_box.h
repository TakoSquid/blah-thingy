#pragma once
#include <functional>
#include <set>
#include "world.h"

namespace BT
{
	class SignalBox : public Component
	{
	public:
		SignalBox();

		void addSignalBox(SignalBox* signal_box);
		void removeSignalBox(SignalBox* signal_box);

		void activate();

		void debug() override;
		const Vector<SignalBox*> get_listeners() const;

		std::function<void(SignalBox* self)> on_signal_action;
	private:
		std::set<SignalBox*> signal_boxes;
	};
}