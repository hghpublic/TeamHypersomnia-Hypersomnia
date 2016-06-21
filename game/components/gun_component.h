#pragma once
#include "misc/timer.h"

#include "game/entity_id.h"

#include "render_component.h"
#include "game/detail/physics_setup_helpers.h"

#include "misc/deterministic_timing.h"
#include "misc/recoil_player.h"

class gun_system;
class processing_system;

namespace components {
	struct gun  {
		enum action_type {
			SINGLE_SHOT,
			BOLT_ACTION,
			SEMI_AUTOMATIC,
			AUTOMATIC
		} action_mode;

		std::pair<float, float> muzzle_velocity;

		float damage_multiplier = 1.f;

		augs::deterministic_timeout timeout_between_shots = augs::deterministic_timeout(100);

		vec2 bullet_spawn_offset;

		float camera_shake_radius = 0.f;
		float camera_shake_spread_degrees = 0.f;

		void shake_camera(cosmos& cosmos, entity_id, float direction, processing_system&);

		components::transform calculate_barrel_transform(components::transform gun_transform);

		// state
		
		bool trigger_pressed = false;

		std::pair<float, float> shell_velocity;
		std::pair<float, float> shell_angular_velocity;

		float shell_spread_degrees = 20.f;

		recoil_player recoil;

		components::transform shell_spawn_offset;
	};
}