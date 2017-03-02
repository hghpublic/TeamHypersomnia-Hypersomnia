#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/fbo.h"
#include "augs/graphics/texture.h"

#include "game/components/transform_component.h"
#include "game/transcendental/entity_id.h"

#include "game/assets/texture_id.h"
#include "augs/misc/timer.h"

namespace augs {
	class texture_atlas;

	namespace graphics {
		class fbo;
	}

	class renderer {
	public:
		struct debug_line {
			debug_line(vec2 a = vec2(), vec2 b = vec2(), rgba col = rgba(255, 255, 255, 255)) : col(col), a(a), b(b) {}

			rgba col;
			vec2 a, b;
		};

		struct line_channel {
			std::vector<debug_line> lines;

			void draw(vec2 a, vec2 b, rgba = white);

			void draw_red(vec2 a, vec2 b);
			void draw_green(vec2 a, vec2 b);
			void draw_blue(vec2 a, vec2 b);
			void draw_yellow(vec2 a, vec2 b);
			void draw_cyan(vec2 a, vec2 b);
		};

		graphics::fbo illuminating_smoke_fbo;
		graphics::fbo smoke_fbo;
		graphics::fbo light_fbo;

		line_channel logic_lines, prev_logic_lines;
		line_channel frame_lines;
		line_channel persistent_lines;
		
		timer line_timer;

		unsigned int position_buffer_id, texcoord_buffer_id, color_buffer_id;
		unsigned int triangle_buffer_id;
		unsigned int special_buffer_id;

		vertex_triangle_buffer triangles;
		vertex_line_buffer lines;
		special_buffer specials;

		unsigned long long triangles_drawn_total = 0;

		bool should_interpolate_debug_lines = false;

		bool debug_draw_colinearization = false;
		bool debug_draw_forces = false;
		bool debug_draw_friction_field_collisions_of_entering = false;
		bool debug_draw_explosion_forces = false;

		float visibility_expansion = 1.0f;
		float max_visibility_expansion_distance = 1000.0f;
		int debug_drawing = 1;

		int draw_visibility = 0;

		int draw_steering_forces = 0;
		int draw_substeering_forces = 0;
		int draw_velocities = 0;

		int draw_avoidance_info = 0;
		int draw_wandering_info = 0;

		int draw_weapon_info = 0;

		void initialize();
		void initialize_fbos(const vec2i screen_size);

		void bind_texture(const graphics::fbo&);
		void bind_texture(const augs::texture&);

		void set_active_texture(const unsigned);

		void fullscreen_quad();
		
		void clear_logic_lines();
		void clear_frame_lines();
		void draw_debug_info(vec2 visible_world_area, components::transform, assets::texture_id tex, std::vector<const_entity_handle> target_entities, float interpolation_ratio);

		void clear_current_fbo();
		void enable_special_vertex_attribute();
		void disable_special_vertex_attribute();
		void call_triangles();
		void call_triangles(const vertex_triangle_buffer&);
		void call_lines();
		void set_viewport(xywhi);
		void push_line(const vertex_line&);
		void push_triangle(const vertex_triangle&);
		void push_triangles(const vertex_triangle_buffer&);
		void push_special_vertex_triangle(augs::special, augs::special, augs::special);

		void clear_special_vertex_data();
		void clear_triangles();
		void clear_lines();

		void default_render(const vec2 visible_world_area);

		size_t get_max_texture_size() const;

		int get_triangle_count() const;
		vertex_triangle& get_triangle(int i);
		std::vector<vertex_triangle>& get_triangle_buffer();

		void clear_geometry();
		static renderer& get_current();
	};
}
