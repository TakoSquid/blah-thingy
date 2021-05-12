#include "signal_box.h"
#include <blah.h>

using namespace Blah;

namespace BT
{
	SignalBox::SignalBox()
		:on_signal_action([](SignalBox* self){
		Log::info("SignalBox at %x activated !", self);
		})
	{

	}

	void SignalBox::addSignalBox(SignalBox* signal_box)
	{
		signal_boxes.insert(signal_box);
	}

	void SignalBox::removeSignalBox(SignalBox* signal_box)
	{
		signal_boxes.erase(signal_box);
	}

	void SignalBox::activate()
	{
		on_signal_action(this);

		for (const auto& sb : signal_boxes)
			sb->activate();

		Log::info("Done for %x", this);
	}
}