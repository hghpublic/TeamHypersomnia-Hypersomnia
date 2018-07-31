#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scene_sounds.h"
#include "test_scenes/test_scene_animations.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"

#include "game/assets/ids/asset_ids.h"

#include "game/components/missile_component.h"
#include "game/components/item_component.h"
#include "game/components/melee_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/explosive_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sender_component.h"
#include "game/components/hand_fuse_component.h"

#include "game/enums/filters.h"

#include "game/detail/inventory/perform_transfer.h"

namespace test_flavours {
	void populate_grenade_flavours(const populate_flavours_input in) {
		auto& flavours = in.flavours;
		auto& caches = in.caches;
		auto& plain_animations = in.plain_animations;

		(void)plain_animations;

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::FORCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDeals damage to [color=red]Health[/color]."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);

			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::FORCE_GRENADE, white);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.6");
			meta.set(item);

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			meta.set(fuse);

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			in.type = adverse_element_type::FORCE;
			in.damage = 88.f;
			in.inner_ring_color = red;
			in.outer_ring_color = orange;
			in.effective_radius = 300.f;
			in.impact_impulse = 550.f;
			in.sound_gain = 1.8f;
			in.sound_effect = to_sound_id(test_scene_sound_id::GREAT_EXPLOSION);

			in.victim_shake.duration_ms = 500.f;
			in.victim_shake.mult = 1.2f;

			explosive.released_image_id = to_image_id(test_scene_image_id::FORCE_GRENADE_RELEASED);
			explosive.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::INTERFERENCE_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDeals damage to [color=orange]Consciousness[/color].\nCauses massive aimpunch."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::INTERFERENCE_GRENADE, white);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.6");
			meta.set(item);

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			meta.set(fuse);

			invariants::explosive explosive; 

			auto& in = explosive.explosion;

			in.damage = 100.f;
			in.inner_ring_color = yellow;
			in.outer_ring_color = orange;
			in.effective_radius = 450.f;
			in.impact_impulse = 2.f;
			in.sound_gain = 2.2f;
			in.sound_effect = to_sound_id(test_scene_sound_id::INTERFERENCE_EXPLOSION);
			in.type = adverse_element_type::INTERFERENCE;

			in.victim_shake.duration_ms = 800.f;
			in.victim_shake.mult = 1.5f;

			explosive.released_image_id = to_image_id(test_scene_image_id::INTERFERENCE_GRENADE_RELEASED);
			explosive.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::PED_GRENADE);

			meta.get<invariants::text_details>().description =
				"Throwable explosive with a one second delay.\nDrains [color=cyan]Personal Electricity[/color].\nIf the subject has [color=turquoise]Electric Shield[/color] enabled,\nthe effect is doubled."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}
			test_flavours::add_sprite(meta, caches, test_scene_image_id::PED_GRENADE, white);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("0.6");
			meta.set(item);

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			meta.set(fuse);

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			in.damage = 88.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = turquoise;
			in.effective_radius = 350.f;
			in.impact_impulse = 2.f;
			in.sound_gain = 2.2f;
			in.sound_effect = to_sound_id(test_scene_sound_id::PED_EXPLOSION);
			in.type = adverse_element_type::PED;
			in.create_thunders_effect = true;

			explosive.released_image_id = to_image_id(test_scene_image_id::PED_GRENADE_RELEASED);
			explosive.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);

			meta.set(explosive);
		}

		{
			auto& meta = get_test_flavour(flavours, test_hand_explosives::BOMB);

			meta.get<invariants::text_details>().description =
				"Can be planted. Deals massive damage nearby."
			;

			{
				invariants::render render_def;
				render_def.layer = render_layer::SMALL_DYNAMIC_BODY;

				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BOMB_1, white);
			test_flavours::add_see_through_dynamic_body(meta);

			invariants::item item;
			item.space_occupied_per_charge = to_space_units("1000");
			item.categories_for_slot_compatibility = { item_category::GENERAL, item_category::SHOULDER_CONTAINER };
			item.wear_sound.id = to_sound_id(test_scene_sound_id::BACKPACK_WEAR);

			meta.set(item);

			invariants::hand_fuse fuse; 
			fuse.release_sound.id = to_sound_id(test_scene_sound_id::GRENADE_THROW);
			fuse.armed_sound.id = to_sound_id(test_scene_sound_id::GRENADE_UNPIN);
			fuse.fuse_delay_ms = 30000.f;
			fuse.override_release_impulse = true;
			fuse.additional_release_impulse = {};
			fuse.set_bomb_vars(2000.f, 10000.f);
			fuse.beep_sound.id = to_sound_id(test_scene_sound_id::BEEP);
			fuse.beep_color = red;
			fuse.beep_time_mult = 0.08f;
			meta.set(fuse);

			invariants::explosive explosive; 

			auto& in = explosive.explosion;
			in.type = adverse_element_type::FORCE;
			in.damage = 348.f;
			in.inner_ring_color = rgba(255, 37, 0, 255);
			in.outer_ring_color = orange;
			in.effective_radius = 500.f;
			in.impact_impulse = 950.f;
			in.sound_gain = 2.f;
			in.sound_effect = to_sound_id(test_scene_sound_id::EXPLOSION);

			in.victim_shake.duration_ms = 700.f;
			in.victim_shake.mult = 1.4f;

			explosive.armed_animation_id = to_animation_id(test_scene_plain_animation_id::BOMB_ARMED);
			explosive.defused_image_id = to_image_id(test_scene_image_id::BOMB_DEFUSED);
			explosive.released_physical_material = to_physical_material_id(test_scene_physical_material_id::GRENADE);

			meta.set(explosive);

			invariants::animation anim;
			anim.id = to_animation_id(test_scene_plain_animation_id::BOMB);
			meta.set(anim);
		}
	}
}
