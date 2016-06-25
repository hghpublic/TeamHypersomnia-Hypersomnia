#pragma once
#include <tuple>

namespace augs {
	template<class... Systems>
	class storage_for_stateful_systems {
		std::tuple<Systems...> systems;
	public:
		storage_for_stateful_systems(Systems&&... args) : systems(std::forward(std::make_tuple(args...))) {}

		template <typename T>
		T& get() {
			return *std::get<std::unique_ptr<T>>(systems);
		}

		template <typename T>
		const T& get() const {
			return *std::get<std::unique_ptr<T>>(systems);
		}
	};
}