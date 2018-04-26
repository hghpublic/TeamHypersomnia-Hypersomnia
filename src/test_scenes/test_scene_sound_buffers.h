#pragma once
#include "test_scenes/test_id_to_pool_id.h"

enum class test_scene_sound_id {
	// GEN INTROSPECTOR enum class test_scene_sound_id
	BILMER2000_MUZZLE,
	ASSAULT_RIFLE_MUZZLE,
	SUBMACHINE_MUZZLE,
	KEK9_MUZZLE,
	ROCKET_LAUNCHER_MUZZLE,
	ELECTRIC_PROJECTILE_FLIGHT,
	ELECTRIC_DISCHARGE_EXPLOSION,
	MISSILE_THRUSTER,

	IMPACT,
	DEATH,
	BULLET_PASSES_THROUGH_HELD_ITEM,

	WIND,
	ENGINE,

	LOW_AMMO_CUE,
	FIREARM_ENGINE,

	CAST_SUCCESSFUL,
	CAST_UNSUCCESSFUL,

	CAST_CHARGING,

	EXPLOSION,
	GREAT_EXPLOSION,

	INTERFERENCE_EXPLOSION,
	PED_EXPLOSION,

	GRENADE_UNPIN,
	GRENADE_THROW,

	ITEM_THROW,

	COLLISION_METAL_WOOD,
	COLLISION_METAL_METAL,
	COLLISION_GRENADE,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto to_sound_id(const test_scene_sound_id id) {
	return to_pool_id<assets::sound_buffer_id>(id);
}
