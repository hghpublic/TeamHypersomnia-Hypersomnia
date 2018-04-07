#pragma once
#include "view/viewables/all_viewables_declarations.h"
#include "view/necessary_resources.h"

namespace augs {
	struct baked_font;
}

struct game_gui_context_dependencies {
	const image_metas_map& image_metas;
	const game_images_in_atlas_map& game_images;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::baked_font& gui_font;
};