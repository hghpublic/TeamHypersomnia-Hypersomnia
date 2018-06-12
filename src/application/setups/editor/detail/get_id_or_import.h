#pragma once
#include "view/viewables/all_viewables_defs.h"
#include "view/maybe_official_path.h"

#include "view/try_load_meta_lua.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_command_input.h"

template <class I, class P>
I get_id_or_import(
	const maybe_official_path<I>& source_path,
	const augs::path_type& project_path,
	const P& definitions, 
	const editor_command_input in,
	const bool has_parent = false
) {
	if (const auto asset_id = ::find_asset_id_by_path(source_path, definitions)) {
		return *asset_id;
	}

	auto& history = in.folder.history;

	{
		using def_type = typename P::mapped_type;

		def_type def;
		def.set_source_path(source_path);

		const auto resolved = def.get_source_path().resolve(project_path);

		::try_load_meta_lua(in.lua, def.meta, resolved);

		auto cmd = create_pathed_asset_id_command<I>(std::move(def));
		cmd.common.has_parent = has_parent;
		history.execute_new(std::move(cmd), in);
	}

	const auto& last_cmd = history.last_command();
	const auto& cmd = std::get<create_pathed_asset_id_command<I>>(last_cmd);

	return cmd.get_allocated_id();
}