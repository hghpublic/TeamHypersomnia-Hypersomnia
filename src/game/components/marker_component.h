#pragma once
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"
#include "augs/math/physics_structs.h"

enum class marker_shape_type {
	// GEN INTROSPECTOR enum class marker_shape_type
	BOX,
	CIRCLE
	// END GEN INTROSPECTOR
};

namespace invariants {
	struct point_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::point_marker
		point_marker_type type = point_marker_type::TEAM_SPAWN;
		// END GEN INTROSPECTOR
	};

	struct area_marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct invariants::area_marker
		area_marker_type type = area_marker_type::BOMBSITE;
		// END GEN INTROSPECTOR
	};
}

struct portal_exit_impulses {
	static constexpr bool json_serialize_in_parent = true;

	// GEN INTROSPECTOR struct portal_exit_impulses
	impulse_amount_def character_exit_impulse = { 5000.0f, impulse_type::ADD_VELOCITY };

	impulse_amount_def object_exit_impulse = { 1000.0f, impulse_type::ADD_VELOCITY };
	impulse_amount_def object_exit_angular_impulse = { 1000.0f, impulse_type::IMPULSE };
	// END GEN INTROSPECTOR

	void set_zero() {
		character_exit_impulse.set_zero();
		object_exit_impulse.set_zero();
		object_exit_angular_impulse.set_zero();
	}
};

namespace components {
	struct marker {
		static constexpr bool reinfer_when_tweaking = true;

		// GEN INTROSPECTOR struct components::marker
		faction_type faction = faction_type::METROPOLIS;
		marker_letter_type letter = marker_letter_type::A;
		marker_shape_type shape = marker_shape_type::BOX;
		// END GEN INTROSPECTOR

	};
}
