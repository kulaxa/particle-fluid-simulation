#include "physics_system.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>

namespace rocket {
	PhysicsSystem::PhysicsSystem(glm::vec2 gravity): gravity(gravity)
	{

	}

	PhysicsSystem::~PhysicsSystem()
	{
	}

	void PhysicsSystem::updatePhysics(float deltaTime, std::vector<RocketGameObject>& gameObjects)
	{
        std::sort(gameObjects.begin(), gameObjects.end(), [](RocketGameObject& a, RocketGameObject& b) {
            if (glm::abs(a.transform2d.translation.y - b.transform2d.translation.y) < 0.0001f) {
                return a.transform2d.translation.x > b.transform2d.translation.x;
            }
            return a.transform2d.translation.y > b.transform2d.translation.y;
        });

        for (auto& gameObject : gameObjects) {
            if (glm::distance(gameObject.transform2d.translation, gameObject.last_position) < 0.001f){
                gameObject.num_of_frames_still++;
            }
            else{
                gameObject.num_of_frames_still = 0;
            }
			if (gameObject.gravityApplied && gameObject.total_force.y/ gameObject.mass < gravity.y) {
				gameObject.total_force += gameObject.mass * gravity * deltaTime;
			}
			if (gameObject.collisionApplied && gameObject.type == RocketGameObjectType::PARTICLE) 
			{
                gameObject.collided_with_x_wall = false;
                gameObject.collided_with_y_wall = false;
                resolveCollisionWithOtherParticles(gameObject, gameObjects, deltaTime);
                resolveCollisionWithOuterWalls(gameObject, deltaTime);

                //resolveCollisionWithOuterWalls(gameObject, deltaTime);

            }
        }
//        for (auto& gameObject : gameObjects) {
//            if (gameObject.collisionApplied && gameObject.type == RocketGameObjectType::PARTICLE)
//            {
////                resolveCollisionWithOtherParticles(gameObject, gameObjects, deltaTime);
//                resolveCollisionWithOuterWalls(gameObject, deltaTime);
//
//
//            }
//        }
        for(auto& gameObject : gameObjects) {
            if(gameObject.num_of_frames_still >  3/deltaTime){
                gameObject.is_still = true;
            }
            else{
                gameObject.is_still = false;
            }
            if(glm::length(gameObject.velocity) < 2.0f)
			    gameObject.velocity +=  gameObject.total_force / gameObject.mass * deltaTime;
            if (gameObject.is_still) {
              //  gameObject.velocity = glm::vec2(0.0f);
            }
            gameObject.last_position = gameObject.transform2d.translation;
			gameObject.transform2d.translation += gameObject.velocity * deltaTime;
            gameObject.collision_resolved = false;

		}
	}

	void PhysicsSystem::resolveCollisionWithOuterWalls(RocketGameObject& gameObject, float deltaTime)
	{
		float radius =  gameObject.radius;
		glm::vec2 future_center = gameObject.transform2d.translation + gameObject.velocity * deltaTime;
        glm::vec2 center = gameObject.transform2d.translation;
        center = future_center;
        const float COLLISION_ENERGY_LOSS = 0.70f;
		const float FRICTION = 0.7f;

		 if (glm::abs(center.x) + radius >= 1.0f) {
			float distance = glm::abs(gameObject.transform2d.translation.x) + radius - 1.0f;
             gameObject.velocity.x *= -COLLISION_ENERGY_LOSS;
             gameObject.collided_with_x_wall = true;
             if (center.x < 0.0f) {
				gameObject.transform2d.translation.x += distance;
			}
			else {
				gameObject.transform2d.translation.x -= distance;
			}
		}

		 if (glm::abs(center.y) + radius >= 1.0f) {
			 gameObject.velocity.y *= -COLLISION_ENERGY_LOSS;
			 float distance = glm::abs(gameObject.transform2d.translation.y) + radius - 1.0f;
             gameObject.collided_with_y_wall = true;

             if (center.y < 0.0f) {
				 gameObject.transform2d.translation.y +=  distance;
			 }
			 else {
				gameObject.transform2d.translation.y -=  distance;
			 }
		}
		
	}

    void PhysicsSystem::resolveCollisionWithOtherParticles(RocketGameObject& gameObject, std::vector<RocketGameObject>& gameObjects, float deltaTime)
	{
		float radius = gameObject.radius;
        glm::vec2 future_center = gameObject.transform2d.translation + gameObject.velocity * deltaTime;
        glm::vec2 center = gameObject.transform2d.translation;
        center = future_center;
        float COLLISION_ENERGY_LOSS = 0.95f;
        auto result = std::vector<RocketGameObject>();
        gameObject.collision_resolved = true;
		for (auto& particle : gameObjects) {
			if (particle.getId() != gameObject.getId()) {
				{

					float otherRadius = particle.radius;
					glm::vec2 future_otherCenter = particle.transform2d.translation + particle.velocity * deltaTime;
                    glm::vec2 otherCenter = particle.transform2d.translation;
                    otherCenter =   future_otherCenter;

                    if (glm::distance(future_center, future_otherCenter) <= radius + otherRadius) {

                        float displacement_X_first = 0;
                        float displacement_Y_first = 0;
                        float displacement_X_second = 1;
                        float displacement_Y_second = 1;

                        bool update_velocity_x = true;
                        bool update_velocity_y = true;

                        float collision_mass_y = gameObject.mass;
                        float collision_mass_x = gameObject.mass;


                        float collision_mass_y_first = particle.mass;
                        float collision_mass_x_first = particle.mass;

                        if(!particle.collision_resolved){
                            if(!gameObject.collided_with_x_wall){
                                displacement_X_first = 0.5f;
                                displacement_X_second = 0.5f;
                            } else{
                                displacement_X_first = 1.0f;
                                displacement_X_second = 0.0f;
                                update_velocity_x = false;
                                collision_mass_x = 1000000.0f;
                            }
                            if(!gameObject.collided_with_y_wall){
                                displacement_Y_first = 0.5f;
                                displacement_Y_second = 0.5f;
                            } else{
                                displacement_Y_first = 1.0f;
                                displacement_Y_second = 0.0f;
                                update_velocity_y = false;
                                collision_mass_y = 1000000.0f;
                            }
                        }
                        else{
                            if(particle.collided_with_x_wall){
                                collision_mass_x_first = 1000000.0f;
                            }
                            if(particle.collided_with_y_wall){
                                collision_mass_y_first = 1000000.0f;
                            }
                            displacement_X_first = 1;
                            displacement_Y_first = 1;
                            displacement_X_second = 0;
                            displacement_Y_second = 0;
                        }


                        float distance = glm::distance(center, otherCenter);
						float overlap = (distance - (radius + otherRadius));
						glm::vec2 v1 = gameObject.velocity;
						glm::vec2 v2 = particle.velocity;
						float m1 = gameObject.mass;
						float m2 = particle.mass;
                        gameObject.transform2d.translation.x -= displacement_X_first * (overlap * (center - otherCenter) / distance).x;
                        gameObject.transform2d.translation.y -= displacement_Y_first * (overlap * (center - otherCenter) / distance).y;


//                        if (!gameObject.collided_with_x_wall)
                            gameObject.velocity.x = ((v1 - (2.0f * m2 / (m1 + collision_mass_x_first)) * glm::dot(v1 - v2, center - otherCenter)
                                / glm::dot(center - otherCenter, center - otherCenter) * (center - otherCenter))).x;
//                        if (!gameObject.collided_with_y_wall)
                            gameObject.velocity.y = ((v1 - (2.0f * m2 / (m1 + collision_mass_y_first)) * glm::dot(v1 - v2, center - otherCenter)
                                / glm::dot(center - otherCenter, center - otherCenter) * (center - otherCenter))).y;

                        gameObject.velocity *= COLLISION_ENERGY_LOSS;

                        particle.transform2d.translation += displacement_X_second * (overlap * (center - otherCenter) / distance).x;
                        particle.transform2d.translation += displacement_Y_second * (overlap * (center - otherCenter) / distance).y;

                        if (update_velocity_x)
                            particle.velocity.x = (v2 - (2.0f * m1 / (collision_mass_x + m2 )) * glm::dot(v2 - v1, otherCenter - center)
                                / glm::dot(otherCenter - center, otherCenter - center) * (otherCenter - center)).x;
                        if (update_velocity_y)
                        particle.velocity.y = (v2 - (2.0f * m1 / (collision_mass_y + m2)) * glm::dot(v2 - v1, otherCenter - center)
                                / glm::dot(otherCenter - center, otherCenter - center) * (otherCenter - center)).y;
                        particle.velocity *= COLLISION_ENERGY_LOSS;

//                        if (particle_collided_with_x_wall) {
//                            particle.collided_with_x_wall = true;
//                        }
//                        if (particle_collided_with_y_wall) {
//                            particle.collided_with_y_wall = true;
//                        }
						return;
					}
				}
			}
		}


		
	}





}
