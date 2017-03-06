#pragma once
#include "augs/misc/pool_id.h"
#include "game/transcendental/types_specification/all_components_declaration.h"

typedef unsigned entity_guid;

namespace augs {
	template<class...>
	class component_aggregate;
}

struct unversioned_entity_id : public augs::unversioned_id<typename put_all_components_into<augs::component_aggregate>::type> {
	typedef augs::unversioned_id<typename put_all_components_into<augs::component_aggregate>::type> base;

	unversioned_entity_id(const base b = base()) : base(b) {}
};

struct entity_id : public augs::pool_id<typename put_all_components_into<augs::component_aggregate>::type> {
	typedef augs::pool_id<typename put_all_components_into<augs::component_aggregate>::type> base;

	entity_id(const base b = base()) : base(b) {}

	operator unversioned_entity_id() const {
		return static_cast<unversioned_entity_id::base>(*static_cast<const base*>(this));
	}
};

struct child_entity_id : entity_id {
	typedef entity_id base;

	child_entity_id(const base b = base()) : base(b) {}

	using base::operator unversioned_entity_id;
};

namespace std {
	template <>
	struct hash<entity_id> {
		std::size_t operator()(const entity_id v) const {
			return hash<entity_id::base>()(v);
		}
	};

	template <>
	struct hash<unversioned_entity_id> {
		std::size_t operator()(const unversioned_entity_id v) const {
			return hash<unversioned_entity_id::base>()(v);
		}
	};
}
