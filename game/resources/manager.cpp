#include "manager.h"
#include <string>
#include "augs/filesystem/file.h"
#include <sstream>
#include "augs/image/font.h"
#include "augs/window_framework/window.h"
#include "game/detail/particle_types.h"

using namespace augs;

namespace assets {
	vec2u get_size(texture_id id) {
		return get_resource_manager().find(id)->texture_maps[texture_map_type::DIFFUSE].original_size_pixels;
	}
}

augs::font_metadata& operator*(const assets::font_id& id) {
	return *get_resource_manager().find(id);
}

bool operator!(const assets::font_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

augs::texture_atlas_entry& operator*(const assets::texture_id& id) {
	return get_resource_manager().find(id)->texture_maps[texture_map_type::DIFFUSE];
}

resources::animation_response& operator*(const assets::animation_response_id& id) {
	return *get_resource_manager().find(id);
}

resources::particle_effect_response& operator*(const assets::particle_effect_response_id& id) {
	return *get_resource_manager().find(id);
}

resources::particle_effect& operator*(const assets::particle_effect_id& id) {
	return *get_resource_manager().find(id);
}

resources::behaviour_tree& operator*(const assets::behaviour_tree_id& id) {
	return *get_resource_manager().find(id);
}

resources::tile_layer& operator*(const assets::tile_layer_id& id) {
	return *get_resource_manager().find(id);
}

augs::sound_buffer& operator*(const assets::sound_buffer_id id) {
	return *get_resource_manager().find(id);
}

bool operator!(const assets::tile_layer_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

bool operator!(const assets::texture_id& id) {
	return get_resource_manager().find(id) == nullptr;
}

template <class T, class K>
auto ptr_if_found(T& container, const K& id) -> decltype(std::addressof(container[id])) {
	const auto it = container.find(id);

	if (it == container.end()) {
		return nullptr;
	}

	return &(*it).second;
}

namespace resources {
	game_image_baked* manager::find(const assets::texture_id id) {
		return ptr_if_found(baked_game_images, id);
	}

	game_font_baked* manager::find(const assets::font_id id) {
		return ptr_if_found(baked_game_fonts, id);
	}

	augs::graphics::texture* manager::find(const assets::atlas_id id) {
		return ptr_if_found(physical_textures, id);
	}

	graphics::shader_program* manager::find(const assets::program_id id) {
		return ptr_if_found(programs, id);
	}

	animation* manager::find(const assets::animation_id id) {
		return ptr_if_found(animations, id);
	}

	animation_response* manager::find(assets::animation_response_id id) {
		return ptr_if_found(animation_responses, id);
	}

	particle_effect_response* manager::find(assets::particle_effect_response_id id) {
		return ptr_if_found(particle_effect_responses, id);
	}

	behaviour_tree* manager::find(assets::behaviour_tree_id id) {
		return ptr_if_found(behaviour_trees, id);
	}

	particle_effect* manager::find(assets::particle_effect_id id) {
		return ptr_if_found(particle_effects, id);
	}

	tile_layer* manager::find(assets::tile_layer_id id) {
		return ptr_if_found(tile_layers, id);
	}

	augs::sound_buffer* manager::find(const assets::sound_buffer_id id) {
		return ptr_if_found(sound_buffers, id);
	}

	sound_response* manager::find(const assets::sound_response_id id) {
		return ptr_if_found(sound_responses, id);
	}

	augs::sound_buffer& manager::create(const assets::sound_buffer_id id) {
		augs::sound_buffer& snd = sound_buffers[id];
		return snd;
	}

	sound_response& manager::create(const assets::sound_response_id id) {
		return sound_responses[id];
	}

	void manager::load_baked_metadata(
		const game_image_requests& images,
		const game_font_requests& fonts,
		const atlases_regeneration_output& atlases
	) {
		for (const auto& requested_image : images) {
			auto& baked_image = baked_game_images[requested_image.first];

			for (size_t i = 0; i < baked_image.texture_maps.size(); ++i) {
				const auto seeked_identifier = requested_image.second.texture_maps[i].filename;
				const bool this_texture_map_is_not_used = seeked_identifier.empty();

				if (this_texture_map_is_not_used) {
					continue;
				}

				for (const auto& a : atlases.metadatas) {
					const auto it = a.second.images.find(seeked_identifier);

					if (it != a.second.images.end()) {
						const auto found_atlas_entry = (*it).second;
						baked_image.texture_maps[i] = found_atlas_entry;

						break;
					}
				}
			}

			baked_image.settings = requested_image.second.settings;
		}

		for (const auto& requested_font : fonts) {
			auto& baked_font = baked_game_fonts[requested_font.first];

			const auto seeked_identifier = requested_font.second.loading_input;

			ensure(seeked_identifier.filename.size() > 0);
			ensure(seeked_identifier.characters.size() > 0);
			ensure(seeked_identifier.pt > 0);

			for (const auto& a : atlases.metadatas) {
				const auto it = a.second.fonts.find(seeked_identifier);

				if (it != a.second.fonts.end()) {
					const auto found_atlas_entry = (*it).second;
					baked_font = found_atlas_entry;

					break;
				}
			}
		}
	}

	animation& manager::create(assets::animation_id id) {
		animation& anim = animations[id];
		return anim;
	}

	animation& manager::create(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms, 
		resources::animation::loop_type loop_mode) {
		
		animation& anim = create(id);
		anim.loop_mode = loop_mode;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i)+1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		return anim;
	}
	
	animation& manager::create_inverse(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms) {
		animation& anim = create(id);
		anim.loop_mode = animation::loop_type::INVERSE;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		return anim;
	}

	animation& manager::create_inverse_with_flip(assets::animation_id id, assets::texture_id first_frame, assets::texture_id last_frame, float frame_duration_ms) {
		animation& anim = create(id);
		anim.loop_mode = animation::loop_type::REPEAT;

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(int(last_frame) - 1); i >= first_frame; i = assets::texture_id(int(i) - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = first_frame; i < last_frame; i = assets::texture_id(int(i) + 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		for (assets::texture_id i = assets::texture_id(int(last_frame) - 1); i >= first_frame; i = assets::texture_id(int(i) - 1)) {
			animation::frame frame;
			frame.duration_milliseconds = frame_duration_ms;
			frame.sprite.set(i);
			frame.sprite.flip_vertically = true;

			anim.frames.push_back(frame);
		}

		return anim;
	}

	animation_response& manager::create(assets::animation_response_id id) {
		animation_response& resp = animation_responses[id];
		return resp;
	}


	particle_effect_response& manager::create(assets::particle_effect_response_id id) {
		auto& resp = particle_effect_responses[id];
		return resp;
	}

	particle_effect& manager::create(assets::particle_effect_id id) {
		auto& resp = particle_effects[id];
		return resp;
	}

	graphics::shader& manager::create(assets::shader_id id, std::string filename, augs::graphics::shader::type type) {
		graphics::shader& sh = shaders[id];
		sh.create(type, get_file_contents(filename));

		return sh;
	}

	graphics::shader_program& manager::create(assets::program_id program_id, assets::shader_id vertex_id, assets::shader_id fragment_id) {
		graphics::shader_program& p = programs[program_id];
		p.create(); 
		p.attach(shaders[vertex_id]);
		p.attach(shaders[fragment_id]);
		p.build();

		return p;
	}

	behaviour_tree& manager::create(assets::behaviour_tree_id id) {
		auto& tree = behaviour_trees[id];
		return tree;
	}

	tile_layer& manager::create(assets::tile_layer_id id) {
		auto& layer = tile_layers[id];
		return layer;
	}

	void manager::destroy_everything() {
		this->~manager();
		new (this) manager;
	}
}

resources::manager& get_resource_manager() {
	return augs::window::glwindow::get_current()->resources;
}