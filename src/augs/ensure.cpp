#include <csignal>

#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/shell.h"

void save_log_and_terminate() {
	if (augs::window::current_exists()) {
		augs::window::get_current().disable_cursor_clipping();
	}
	
	const auto logs = program_log::get_current().get_complete();
	const auto failure_log_path = augs::path_type(LOG_FILES_DIR "ensure_failed_debug_log.txt");

	augs::save_as_text(failure_log_path, logs);

	augs::open_text_editor(failure_log_path.string());

#if !IS_PRODUCTION_BUILD
	#if PLATFORM_WINDOWS
	__debugbreak();
	#elif PLATFORM_UNIX
	std::raise(SIGINT);	
	#endif
#else
	std::terminate();
#endif
}
