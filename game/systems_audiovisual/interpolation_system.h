#pragma once
#include "augs/misc/pooled_object_id.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>

#include "game/components/transform_component.h"
#include "augs/misc/delta.h"

class interpolation_system {
	bool enabled = true;

public:
	struct cache {
		components::transform recorded_place_of_birth;
		components::transform interpolated_transform;
		unsigned recorded_version = 0;
		float rotational_slowdown_multiplier = 1.f;
		float positional_slowdown_multiplier = 1.f;
	};

	std::vector<cache> per_entity_cache;
	float interpolation_speed = 525.f;

	void integrate_interpolated_transforms(
		const cosmos&, 
		const augs::delta delta, 
		const augs::delta fixed_delta_for_slowdowns
	);

	components::transform get_interpolated(const const_entity_handle) const;
	components::transform& get_interpolated(const const_entity_handle);

	void set_interpolation_enabled(const bool flag);

	void reserve_caches_for_entities(const size_t);

	cache& get_data(const entity_id);
};