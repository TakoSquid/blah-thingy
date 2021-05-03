#include "debugSolid.h"
#include "collider.h"

void BT::DebugSolid::render(Batch& batch)
{
	auto collider = get<Collider>();

	static const Color color = Color(245, 98, 182);

	batch.push_matrix(Mat3x2::create_translation(entity()->position));

	batch.rect(collider->get_rect(), color);

	batch.pop_matrix();
}