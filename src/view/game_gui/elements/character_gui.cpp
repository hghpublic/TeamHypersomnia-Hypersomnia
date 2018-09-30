#include "augs/ensure.h"
#include "view/game_gui/game_gui_element_location.h"
#include "view/game_gui/elements/character_gui.h"

#include "augs/gui/text/printer.h"
#include "augs/templates/identity_templates.h"

#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "view/game_gui/elements/game_gui_root.h"
#include "view/game_gui/elements/drag_and_drop_target_drop_item.h"
#include "view/game_gui/elements/item_button.h"
#include "view/game_gui/elements/slot_button.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/detail/visible_entities.h"
#include "view/game_gui/elements/drag_and_drop.h"
#include "game/components/container_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"
#include "game/components/render_component.h"
#include "view/rendering_scripts/draw_entity.h"
#include "view/game_gui/game_gui_system.h"

#include "view/game_gui/viewing_game_gui_context_dependencies.h"
#include "view/game_gui/elements/drag_and_drop.h"
#include "view/audiovisual_state/aabb_highlighter.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/viewables/image_in_atlas.h"
#include "view/viewables/images_in_atlas_map.h"
#include "game/enums/filters.h"

#include "game/detail/describers.h"

#include "augs/drawing/drawing.h"
#include "augs/templates/get_by_dynamic_id.h"
#include "game/detail/inventory/wielding_result.h"
#include "game/detail/spells/spell_structs.h"
#include "game/detail/entity_handle_mixins/make_wielding_transfers.hpp"
#include "game/detail/inventory/wielding_setup.hpp"

#define LOG_HOTBAR 0

template <class... Args>
void HOT_LOG(Args&&... args) {
#if LOG_HOTBAR
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_HOTBAR
#define HOT_LOG_NVPS LOG_NVPS
#else
#define HOT_LOG_NVPS HOT_LOG
#endif

using namespace augs;
using namespace augs::gui;
using namespace augs::gui::text;

xywh character_gui::get_rectangle_for_slot_function(const slot_function f) {
	const auto unit = 70;

	switch (f) {
		case slot_function::SECONDARY_HAND: return xywh(-unit*3, -unit, 33, 33);

		case slot_function::PRIMARY_HAND: return xywh(-unit*3, 0, 33, 33);
		case slot_function::PERSONAL_DEPOSIT: return xywh(-unit * 1, 0, 33, 33);
		case slot_function::SHOULDER: return xywh(0, 0, 33, 33);
		case slot_function::BELT: return xywh(unit, 0, 33, 33);
		case slot_function::BACK: return xywh(unit * 2, 0, 33, 33);
		case slot_function::TORSO_ARMOR: return xywh(unit * 3, 0, 33, 33);

		case slot_function::ITEM_DEPOSIT: return xywh(0, 0, 33, 33);

		case slot_function::GUN_DETACHABLE_MAGAZINE: return xywh(0, unit, 33, 33);
		case slot_function::GUN_CHAMBER: return xywh(0, -unit, 33, 33);
		case slot_function::GUN_MUZZLE: return xywh(-unit, 0, 33, 33);
		default: ensure(false);
	}

	ensure(false);

	return xywh(0, 0, 0, 0);
}

vec2i character_gui::get_initial_position_for(const vec2i screen_size, const drag_and_drop_target_drop_item&) const {
	return vec2i(screen_size.x - 150, 30);
}

const wielding_setup& character_gui::get_current_hotbar_selection_setup() const {
	return last_setups[current_hotbar_selection_setup_index];
}

wielding_setup character_gui::get_setup_from_button_indices(
	const const_entity_handle gui_entity,
	const int hotbar_button_index_for_primary_selection,
	const int hotbar_button_index_for_secondary_selection
) const {
	wielding_setup output;

	const auto primary = hotbar_button_index_for_primary_selection;
	const auto secondary = hotbar_button_index_for_secondary_selection;

	if (primary != -1) {
		output.hand_selections[0] = hotbar_buttons[static_cast<size_t>(primary)].get_assigned_entity(gui_entity);
	}

	if (secondary != -1) {
		output.hand_selections[1] = hotbar_buttons[static_cast<size_t>(secondary)].get_assigned_entity(gui_entity);
	}

	return output;
}

bool character_gui::hotbar_assignment_exists_for(const entity_id id) const{
	for (const auto& h : hotbar_buttons) {
		if (h.last_assigned_entity == id) {
			return true;
		}
	}

	return false;
}

void character_gui::clear_hotbar_selection_for_item(
	const const_entity_handle /* gui_entity */,
	const const_entity_handle item_entity
) {
	for (auto& h : hotbar_buttons) {
		if (h.last_assigned_entity == item_entity) {
			h.last_assigned_entity.unset();
		}
	}
}

void character_gui::clear_hotbar_button_assignment(
	const size_t button_index,
	const const_entity_handle /* gui_entity */
) {
	hotbar_buttons[button_index].last_assigned_entity.unset();
}

void character_gui::assign_item_to_hotbar_button(
	const size_t button_index,
	const const_entity_handle gui_entity,
	const const_entity_handle item
) {
	clear_hotbar_selection_for_item(gui_entity, item);

	{
		const auto item_capability = item.get_owning_transfer_capability();

		if (item_capability != gui_entity) {
			LOG("Warning! Assigned entity's owning capability is not the gui subject (%x != %x)!", item_capability, gui_entity);
		}
	}

	hotbar_buttons[button_index].last_assigned_entity = item;
}

void character_gui::assign_item_to_first_free_hotbar_button(
	const const_entity_handle gui_entity,
	const const_entity_handle item_entity,
	const bool from_the_right
) {
	/* const auto categories = item_entity.get<invariants::item>().categories_for_slot_compatibility; */

	clear_hotbar_selection_for_item(gui_entity, item_entity);

	auto try_assign = [&](const size_t n) {
		if (hotbar_buttons[n].get_assigned_entity(gui_entity).dead()) {
			assign_item_to_hotbar_button(n, gui_entity, item_entity);

			return true;
		}

		return false;
	};

	if (from_the_right) {
		for (int i = hotbar_buttons.size() - 1; i >= 0; --i) {
			if (try_assign(i)) {
				return;
			}
		}
	}
	else {
		for (size_t i = 0; i < hotbar_buttons.size(); ++i) {
			if (try_assign(i)) {
				return;
			}
		}
	}
}

wielding_setup character_gui::make_wielding_setup_for_previous_hotbar_selection_setup(
	const const_entity_handle gui_entity
) {
	auto& current_setup_index = current_hotbar_selection_setup_index;
	const auto& cosm = gui_entity.get_cosmos();

	const auto prev_idx = 1 - current_setup_index;

	HOT_LOG("Q. Current: %x Prev: %x", current_setup_index, prev_idx);

	const auto previous_setup = last_setups[prev_idx].make_viable_setup(gui_entity);

	if (previous_setup == wielding_setup::from_current(gui_entity)) {
		/* Previous is identical so wield first item from hotbar */

		HOT_LOG("Same setup:");

		HOT_LOG_NVPS(cosm[previous_setup.hand_selections[0]]);
		HOT_LOG_NVPS(cosm[previous_setup.hand_selections[1]]);

		const auto try_wielding_item_from_hotbar_button_no = [&](const std::size_t hotbar_button_index) {
			std::optional<wielding_setup> output;

			const auto tried_setup = get_setup_from_button_indices(gui_entity, static_cast<int>(hotbar_button_index), -1);
			const auto candidate_entity = cosm[tried_setup.hand_selections[0]];

			if (candidate_entity.alive()) {
				if (!is_clothing(candidate_entity.get<invariants::item>().categories_for_slot_compatibility)) {
					const bool finally_found_differing_setup = !(tried_setup == previous_setup);

					if (finally_found_differing_setup) {
						output = tried_setup;
					}
				}
			}

			return output;
		};

		for (std::size_t i = 0; i < hotbar_buttons.size(); ++i) {
			if (const auto wielding = try_wielding_item_from_hotbar_button_no(i)) {
				return *wielding;
			}
		}

		return {};
	}

	HOT_LOG("Different setups, standard request.");

	current_setup_index = prev_idx;
	return previous_setup;
}

void character_gui::save_setup(const wielding_setup now_actual_setup) {
	HOT_LOG("Saving setup to %x", current_hotbar_selection_setup_index);
	last_setups[current_hotbar_selection_setup_index] = now_actual_setup;
}

void character_gui::push_setup(const wielding_setup new_setup) {
	auto& current = current_hotbar_selection_setup_index;
	current = 1 - current;

	HOT_LOG("Pushing setup to %x", current);
	last_setups[current] = new_setup;
}

void character_gui::draw_cursor_with_tooltip(
	const viewing_game_gui_context context,
	const bool draw_cursor
) const {
	const auto drag_amount = context.get_rect_world().current_drag_amount;
	const auto& manager = context.get_necessary_images();

	const auto& rect_world = context.get_rect_world();
	const auto output = context.get_output();
	auto gui_cursor = assets::necessary_image_id::GUI_CURSOR;
	auto gui_cursor_color = white;

	auto get_gui_crosshair_position = [&context]() {
		return context.get_input_state().mouse.pos;
	};

	auto gui_cursor_position = get_gui_crosshair_position();
	const auto screen_size = context.screen_size;

	auto get_tooltip_position = [&]() {
		return get_gui_crosshair_position() + manager.at(gui_cursor).get_original_size();
	};

	auto draw_cursor_hint = [&](const auto& hint_text, const vec2i cursor_position, const vec2i cursor_size) {
		const auto text = formatted_string(hint_text, context.get_gui_font());

		vec2i bg_sprite_size = get_text_bbox(text);
		bg_sprite_size.y = std::max(cursor_size.y, bg_sprite_size.y);
		bg_sprite_size.x += cursor_size.x;

		const auto hint_rect = ltrbi(cursor_position, bg_sprite_size).snap_to_bounds(ltrbi(vec2i(0, 0), screen_size - vec2i(1, 1)));

		output.aabb_with_border(
			hint_rect,
			rgba(0, 0, 0, 180),
			rgba(255, 255, 255, 35)
		);

		augs::gui::text::print_stroked(
			output,
			hint_rect.get_position() + vec2i(cursor_size.x + 2, 0),
			text
		);

		return hint_rect.get_position();
	};

	auto get_absolute_pos = [&](const game_gui_element_location l) {
		return context.get_tree_entry(l).get_absolute_pos();
	};

	const bool is_dragging = rect_world.is_currently_dragging();

	if (!is_dragging) {
		if (context.alive(rect_world.rect_hovered)) {
			gui_cursor = assets::necessary_image_id::GUI_CURSOR_HOVER;
		}

		draw_tooltip_from_hover_or_world_highlight(context, get_tooltip_position());
	}
	else {
		const auto drag_result = prepare_drag_and_drop_result(context, rect_world.rect_held_by_lmb, rect_world.rect_hovered);

		if (drag_result.has_value()) {
			std::visit([&](const auto& transfer_data) {
				using T = remove_cref<decltype(transfer_data)>;

				if constexpr (std::is_same_v<T, unfinished_drag_of_item>) {
					const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });
					const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

					if (hotbar_location != nullptr) {
						const auto drawn_pos = drag_amount + get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button);

						dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, drawn_pos);

						gui_cursor = assets::necessary_image_id::GUI_CURSOR_MINUS;
						gui_cursor_color = red;

						gui_cursor_position = draw_cursor_hint("Clear assignment", get_gui_crosshair_position(), manager.at(gui_cursor).get_original_size());
					}
					else {
						const auto drawn_pos = drag_amount;

						dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, drawn_pos);
						dragged_item_button->draw_grid_border_ghost(context, dragged_item_button, drawn_pos);
					}

					const auto item = context.get_cosmos()[dragged_item_button.get_location().item_id].get<components::item>();

					if (item.get_charges() > 1) {
						const auto gui_cursor_size = manager.at(gui_cursor).get_original_size();

						const auto charges_text = std::to_string(dragged_charges);

						augs::gui::text::print_stroked(
							output,
							get_gui_crosshair_position() + vec2i(0, int(gui_cursor_size.y)),
							{ charges_text, { context.get_gui_font(), white } }
						);
					}
				}
				else if constexpr (std::is_same_v<T, drop_for_hotbar_assignment>) {
					const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.item_id });

					const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

					if (hotbar_location != nullptr) {
						const auto drawn_pos = drag_amount + get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button);

						dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, drawn_pos);
					}
					else {
						/* TODO: Why was it left here? const auto drawn_pos = drag_amount; */
						dragged_item_button->draw_complete_dragged_ghost(context, dragged_item_button, drag_amount);
					}

					if (!(transfer_data.source_hotbar_button_id == transfer_data.assign_to)) {
						gui_cursor = assets::necessary_image_id::GUI_CURSOR_ADD;
						gui_cursor_color = green;
					}

					gui_cursor_position = draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), manager.at(gui_cursor).get_original_size());
				}
				else if constexpr (std::is_same_v<T, drop_for_item_slot_transfer>) {
					const auto& dragged_item_button = context.dereference_location(item_button_in_item{ transfer_data.simulated_transfer.item });

					const auto hotbar_location = context.dereference_location(transfer_data.source_hotbar_button_id);

					if (hotbar_location != nullptr) {
						dragged_item_button->draw_complete_dragged_ghost(
							context, 
							dragged_item_button,
							drag_amount + get_absolute_pos(hotbar_location) - get_absolute_pos(dragged_item_button)
						);
					}
					else {
						dragged_item_button->draw_complete_dragged_ghost(
							context, 
							dragged_item_button, 
							drag_amount
						);
					}

					const auto& transfer_result = transfer_data.result;
					const auto& transfer_result_type = transfer_result.result;

					if (transfer_result.is_successful()) {
						if (transfer_data.result.relation == capability_relation::DROP) {
							gui_cursor = assets::necessary_image_id::GUI_CURSOR_MINUS;
							gui_cursor_color = red;
						}
						else {
							gui_cursor = assets::necessary_image_id::GUI_CURSOR_ADD;
							gui_cursor_color = green;
						}
					}
					else if (transfer_result_type != item_transfer_result_type::THE_SAME_SLOT) {
						gui_cursor = assets::necessary_image_id::GUI_CURSOR_ERROR;
						gui_cursor_color = red;
					}

					gui_cursor_position = draw_cursor_hint(transfer_data.hint_text, get_gui_crosshair_position(), manager.at(gui_cursor).get_original_size());
				}
				else {
					static_assert(always_false_v<T>);
				}
			}, drag_result.value());
		}
	}

	if (draw_cursor) {
		output.aabb_lt(manager.at(gui_cursor), vec2(gui_cursor_position), gui_cursor_color);
	}
}

void character_gui::draw_tooltip_from_hover_or_world_highlight(
	const viewing_game_gui_context context,
	const vec2i tooltip_pos
) const {
	const auto& rect_world = context.get_rect_world();
	const auto& cosm = context.get_cosmos();
	const auto gui_entity = context.get_subject_entity();
	const auto output = context.get_output();
	const auto screen_size = context.screen_size;

	formatted_string tooltip_text;

	const auto description_style = text::style(context.get_gui_font(), vslightgray);

	const auto hovered = rect_world.rect_hovered;

	if (context.alive(hovered)) {
		context(
			hovered,
			[&](const auto dereferenced) {
				const auto location = dereferenced.get_location();
				using T = std::remove_const_t<decltype(location)>;

				if constexpr(std::is_same_v<T, item_button_in_item>) {
					tooltip_text = text::from_bbcode(get_bbcoded_entity_details(cosm[location.item_id]), description_style);
				}
				else if constexpr(std::is_same_v<T, slot_button_in_container>) {
					tooltip_text = text::from_bbcode(get_bbcoded_slot_details(cosm[location.slot_id]), context.get_gui_font());
				}
				else if constexpr(std::is_same_v<T, hotbar_button_in_character_gui>) {
					const auto assigned_entity = dereferenced->get_assigned_entity(gui_entity);

					if (assigned_entity.alive()) {
						tooltip_text = text::from_bbcode(get_bbcoded_entity_details(assigned_entity), description_style);
					}
					else {
						tooltip_text = { "Empty slot", description_style };
					}
				}
				else if constexpr(std::is_same_v<T, value_bar_in_character_gui>) {
					tooltip_text = text::from_bbcode(
						dereferenced->get_description_for_hover(context, dereferenced),
						description_style
					);
				}
				else if constexpr(std::is_same_v<T, action_button_in_character_gui>) {
					const auto bound_spell = action_button::get_bound_spell(context, dereferenced);

					if (bound_spell.is_set()) {
						tooltip_text = bound_spell.dispatch(
							[&](auto s) {
								using S = decltype(s);
								const auto& spell_data = std::get<S>(cosm.get_common_significant().spells);

								return text::from_bbcode(
									get_bbcoded_spell_description(
										gui_entity,
										spell_data
									),
									description_style
								);
							}
						);
					}
				}
				else if constexpr(std::is_same_v<T, game_gui_root_in_context>) {

				}
				else if constexpr(std::is_same_v<T, drag_and_drop_target_drop_item_in_character_gui>) {

				}
				else {
					static_assert(always_false<T>::value);
				}
			}
		);
	}
	else {
		const auto eye = context.get_camera_eye();
		const auto cone = camera_cone(eye, screen_size);

		const auto world_cursor_pos = cone.to_world_space(context.get_input_state().mouse.pos);

		const auto hovered = cosm[get_hovered_world_entity(cosm, world_cursor_pos)];

		if (hovered.alive()) {
			if (context.dependencies.settings.draw_aabb_highlighter) {
				context.get_world_hover_highlighter().draw({
					output, 
					hovered, 
					context.get_interpolation_system(), 
					context.get_camera_eye(),
					screen_size
				});
			}

			tooltip_text = from_bbcode(
				get_bbcoded_entity_details(hovered), 
				style(context.get_gui_font(), vslightgray)
			);
		}
	}

	if (tooltip_text.size() > 0) {
		const auto tooltip_rect = ltrb(
			tooltip_pos, 
			get_text_bbox(tooltip_text) + vec2(10.f, 8.f)).snap_to_bounds(ltrb(vec2(0, 0), screen_size - vec2(1, 1))
		);

		output.aabb_with_border(
			tooltip_rect,
			rgba(0, 0, 0, 180),
			rgba(255, 255, 255, 35)
		);

		text::print(
			output,
			tooltip_rect.get_position() + vec2i(5, 4),
			tooltip_text
		);
	}
}

entity_id character_gui::get_hovered_world_entity(const cosmos& cosm, const vec2 world_cursor_position) {
	return ::get_hovered_world_entity(
		cosm,
		world_cursor_position,
		true_returner(),
		aabb_highlighter::get_filter()
	);
}