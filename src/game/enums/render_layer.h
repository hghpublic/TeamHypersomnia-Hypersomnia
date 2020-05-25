#pragma once

enum class render_layer {
	// GEN INTROSPECTOR enum class render_layer
	INVALID,

	GROUND,

	PLANTED_ITEMS,
	SOLID_OBSTACLES,
	ITEMS_ON_GROUND,
	SENTIENCES,
	FOREGROUND,
	FOREGROUND_GLOWS,

	DIM_WANDERING_PIXELS,
	CONTINUOUS_SOUNDS,
	CONTINUOUS_PARTICLES,
	ILLUMINATING_WANDERING_PIXELS,
	LIGHTS,
	AREA_MARKERS,
	POINT_MARKERS,
	CALLOUT_MARKERS,

	COUNT
	// END GEN INTROSPECTOR
};