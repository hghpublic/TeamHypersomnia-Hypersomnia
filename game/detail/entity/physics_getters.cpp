#include "game/entity_handle.h"
#include "game/components/physics_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/cosmos.h"
#include "physics_getters.h"

template <bool C, class D>
D physics_getters<C, D>::get_owner_friction_ground() const {
	auto& self = *static_cast<const D*>(this);
	return self.get_cosmos()[get_owner_body_entity().get<components::special_physics>().owner_friction_ground];
}

template <bool C, class D>
D physics_getters<C, D>::get_owner_body_entity() const {
	auto& self = *static_cast<const D*>(this);
	auto& cosmos = self.get_cosmos();

	if (self.has<components::fixtures>()) return cosmos[self.get<components::fixtures>().get_owner_body()];
	else if (self.has<components::physics>()) return self;
	return cosmos[entity_id()];
}

// explicit instantiation
template class physics_getters<false, basic_entity_handle<false>>;
template class physics_getters<true, basic_entity_handle<true>>;