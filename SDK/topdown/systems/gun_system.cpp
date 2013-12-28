#include "stdafx.h"
#include "gun_system.h"
#include "entity_system/world.h"
#include "../messages/intent_message.h"
#include "../messages/animate_message.h"
#include "../messages/particle_burst_message.h"
#include "../messages/damage_message.h"
#include "../messages/destroy_message.h"

#include "../components/render_component.h"
#include "../components/physics_component.h"
#include "../components/camera_component.h"
#include "../components/damage_component.h"
#include "../components/particle_group_component.h"
#include "../components/chase_component.h"

#include "../systems/physics_system.h"
#include "../systems/render_system.h"

#include "../game/body_helper.h"

gun_system::gun_system() : generator(device()) {}

void components::gun::transfer_barrel_smoke(gun& another) {
	auto& this_group =            barrel_smoke.target_barrel_smoke_group->get<components::particle_group>().stream_slots;
	auto& another_group = another.barrel_smoke.target_barrel_smoke_group->get<components::particle_group>().stream_slots;

	another_group.resize(this_group.size());
	
	for (size_t i = 0; i < this_group.size(); ++i) {
		another_group[i] = this_group[i];
		this_group[i].stop_streaming();
	}
}

void gun_system::add(entity* e) {
	auto& gun = e->get<components::gun>();
	gun.barrel_smoke.target_barrel_smoke_group.set(&e->owner_world.create_entity());

	gun.barrel_smoke.target_barrel_smoke_group->add(components::transform());
	gun.barrel_smoke.target_barrel_smoke_group->add(components::particle_group()).stream_slots[0].destroy_when_empty = false;
	gun.barrel_smoke.target_barrel_smoke_group->add(components::chase());
	gun.barrel_smoke.target_barrel_smoke_group->add(components::render());

	return processing_system::add(e);
}

void gun_system::remove(entity* e) {
	e->owner_world.post_message(messages::destroy_message(e->get<components::gun>().barrel_smoke.target_barrel_smoke_group));
	return processing_system::remove(e);
}

void gun_system::process_events(world& owner) {
	auto events = owner.get_message_queue<messages::intent_message>();

	for (auto it : events) {
		if (it.intent == messages::intent_message::intent_type::SHOOT) {
			it.subject->get<components::gun>().trigger = it.state_flag;
		}
	}
}

bool components::gun::can_drop() const {
	return !is_melee || (is_melee && !is_swinging && shooting_timer.get<std::chrono::milliseconds>() >= shooting_interval_ms);
}

void gun_system::process_entities(world& owner) {
	auto& physics_sys = owner.get_system<physics_system>();
	auto& render = owner.get_system<render_system>();

	for (auto it : targets) {
		auto& gun_transform = it->get<components::transform>().current;
		auto& gun = it->get<components::gun>();

		auto get_damage = [&gun, this](){
			return std::uniform_real_distribution<float>(gun.bullet_damage.first, gun.bullet_damage.second)(generator);
		};

		auto since_shot = gun.shooting_timer.get<std::chrono::milliseconds>();

		if (gun.is_melee) {
			if (since_shot > gun.swing_duration) {
				gun.is_swinging = false;
			}

			if (!gun.is_swinging && gun.trigger && since_shot >= gun.swing_interval_ms) {
				gun.is_swinging = true;

				messages::animate_message msg;
				msg.animation_type = messages::animate_message::animation::SWING;
				msg.preserve_state_if_animation_changes = false;
				msg.change_animation = true;
				msg.change_speed = true;
				msg.speed_factor = 1.f;
				msg.subject = it;
				msg.message_type = messages::animate_message::type::START;
				msg.animation_priority = 1;

				owner.post_message(msg);
				
				gun.shooting_timer.reset();
			}

			if (gun.is_swinging) {
				std::vector<b2Vec2> query_vertices;
				query_vertices.resize(gun.query_vertices + 1);
				query_vertices[gun.query_vertices] = gun_transform.pos;

				for (int i = 0; i < gun.query_vertices; ++i) {
					query_vertices[i] = gun_transform.pos + vec2<>::from_degrees(
						gun_transform.rotation + gun.swing_angular_offset - (gun.swing_angle / 2)
						+ i * (gun.swing_angle / (gun.query_vertices - 1))
						) * gun.swing_radius;
				}

				query_vertices[gun.query_vertices] = gun_transform.pos;

				if (render.draw_weapon_info)
					for (size_t i = 0; i < query_vertices.size(); ++i) {
						render.lines.push_back(render_system::debug_line(query_vertices[i], query_vertices[(i + 1) % query_vertices.size()]));
					}

				for (auto& v : query_vertices)
					v *= PIXELS_TO_METERSf;

				b2PolygonShape swing_query;
				swing_query.Set(query_vertices.data(), query_vertices.size());

				auto hit_bodies = physics_sys.query_shape(&swing_query, &gun.bullet_body.filter, it).bodies;

				for (auto hit_body : hit_bodies) {
					auto target_entity = reinterpret_cast<entity*>(hit_body->GetUserData());
					auto target_transform = target_entity->get<components::transform>().current;
					auto ray_output = physics_sys.ray_cast_px(gun_transform.pos, target_transform.pos, gun.bullet_body.filter, it);

					if (!ray_output.hit || (ray_output.hit && ray_output.what_entity == target_entity)) {
						/* apply damage to the entity */
						vec2<> impact_pos = ray_output.hit ? ray_output.intersection : target_transform.pos;

						messages::damage_message damage_msg;
						damage_msg.subject = target_entity;
						damage_msg.amount = get_damage();
						damage_msg.impact_velocity = impact_pos - gun_transform.pos;

						messages::particle_burst_message burst_msg;
						burst_msg.subject = target_entity;
						burst_msg.pos = impact_pos;
						burst_msg.rotation = (-damage_msg.impact_velocity).get_degrees();
						burst_msg.type = messages::particle_burst_message::burst_type::BULLET_IMPACT;

						owner.post_message(damage_msg);
						owner.post_message(burst_msg);
					}
				}
			}
		}
		else if (!gun.reloading &&
			gun.current_rounds > 0 &&
			gun.trigger &&
			since_shot >= gun.shooting_interval_ms) {

				messages::animate_message msg;
				msg.animation_type = messages::animate_message::animation::SHOT;
				msg.preserve_state_if_animation_changes = false;
				msg.change_animation = true;
				msg.change_speed = true;
				msg.speed_factor = 1.f;
				msg.subject = it;
				msg.message_type = messages::animate_message::type::START;
				msg.animation_priority = 1;

				owner.post_message(msg);

				if (gun.target_camera_to_shake) {
					vec2<> shake_dir;
					shake_dir.set_from_degrees(std::uniform_real_distribution<float>(
						gun_transform.rotation - gun.shake_spread_degrees,
						gun_transform.rotation + gun.shake_spread_degrees)(generator));
				
					gun.target_camera_to_shake->get<components::camera>().last_interpolant += shake_dir * gun.shake_radius;
				}

				if (!gun.is_automatic)
					gun.trigger = false;

				--gun.current_rounds;

				/* place bullets near the very barrel */
				auto new_transform = gun_transform;
				new_transform.pos += vec2<>(gun.bullet_distance_offset).rotate(gun_transform.rotation, vec2<>());

				auto result = physics_sys.ray_cast_px(gun_transform.pos, new_transform.pos, gun.bullet_body.filter, it);
				
				if (result.hit)
					new_transform.pos = result.intersection;

				messages::particle_burst_message burst;
				burst.pos = new_transform.pos;
				burst.rotation = gun_transform.rotation;
				burst.subject = it;
				burst.type = messages::particle_burst_message::burst_type::WEAPON_SHOT;
				burst.target_group_to_refresh = gun.barrel_smoke.target_barrel_smoke_group;

				owner.post_message(burst);
 
				for (unsigned i = 0; i < gun.bullets_once; ++i) {
					entity& new_bullet = owner.create_entity();

					/* randomize bullet direction taking spread into account */
					vec2<> vel(vec2<>::from_degrees(
						std::uniform_real_distribution<float> (
						gun_transform.rotation - gun.spread_degrees,
						gun_transform.rotation + gun.spread_degrees)(generator)));

					new_transform.rotation = vel.get_degrees();
					/* add randomized speed to bullet taking velocity variation into account */
					vel *= std::uniform_real_distribution<float> (
						gun.bullet_speed.first,
						gun.bullet_speed.second)(generator) * PIXELS_TO_METERSf;

					components::damage damage;
					/* randomize damage */
					damage.amount = get_damage();
					damage.sender = it;
					damage.max_distance = gun.max_bullet_distance;
					damage.starting_point = new_transform.pos;

					/* add components that make up a bullet */
					new_bullet.add(components::transform(new_transform));
					new_bullet.add(damage);
					new_bullet.add(gun.bullet_render);
					topdown::create_physics_component(gun.bullet_body, new_bullet, b2_dynamicBody);

					/* bullet's physics settings */
					auto body = new_bullet.get<components::physics>().body;
					body->SetLinearVelocity(vel);
					body->SetAngularVelocity(0);
					body->SetBullet(true);
				}

				gun.shooting_timer.reset();
		}

		if (gun.current_rounds == 0) gun.reloading = true;
	}
}