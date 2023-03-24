#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_marker_resource.h"
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"

struct editor_point_marker_node_editable {
	// GEN INTROSPECTOR struct editor_point_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_area_marker_node_editable {
	// GEN INTROSPECTOR struct editor_area_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	vec2i size = { 256, 256 };
	// END GEN INTROSPECTOR
};

struct editor_point_marker_node : editor_node_base<
	editor_point_marker_resource,
	editor_point_marker_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Area marker";
	}
};

struct editor_area_marker_node : editor_node_base<
	editor_area_marker_resource,
	editor_area_marker_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Point marker";
	}
};