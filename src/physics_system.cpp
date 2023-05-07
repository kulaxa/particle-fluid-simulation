#include "physics_system.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <chrono>

namespace rocket {
    PhysicsSystem::PhysicsSystem(glm::vec2 gravity) : gravity(gravity) {
        grid = Grid(8, 8);
    }

    PhysicsSystem::~PhysicsSystem() {
    }

    void PhysicsSystem::updatePhysics(float deltaTime, std::vector<RocketGameObject> &gameObjects) {
//        std::sort(gameObjects.begin(), gameObjects.end(), [](RocketGameObject &a, RocketGameObject &b) {
//            return a.transform2d.translation.y < b.transform2d.translation.y;
//        });
//        std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();

        grid.updateGrid(gameObjects);
//        std::chrono::high_resolution_clock::time_point end_time = std::chrono::high_resolution_clock::now();
//        debugInfo.update_grid_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time-start_time);

//        start_time = std::chrono::high_resolution_clock::now();
        grid.resolveCollisionsWithWalls(gameObjects);
//        end_time = std::chrono::high_resolution_clock::now();
//        debugInfo.resolve_collisions_with_walls_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time-start_time);

//        start_time = std::chrono::high_resolution_clock::now();
        uint32_t  collision_count = grid.resolveCollisions(gameObjects);
        grid.clearResolvedCells();

        collision_count += grid.resolveCollisions(gameObjects);
        grid.clearResolvedCells();

//
//        grid.updatePositionsFromObjectToGrid(gameObjects);
//        grid.updateObstacleCellsToSolid(gameObjects);
//
//        grid.transferVelocities(true, 0.65f);
//        grid.updateParticleDensity();
//        grid.solveIncompressibility(1, deltaTime, 0.85);
//        grid.updatePositionsFromGridToObject(gameObjects);
//        grid.updateObstacleCellsToAir(gameObjects);
        for (auto &gameObject: gameObjects) {
            if (gameObject.gravityApplied) {
                gameObject.acceleration += gravity * 0.001f;
            }
            if(gameObject.type == rocket::RocketGameObjectType::OBSTACLE) {
                gameObject.obstacleGridPositions.clear();
            }
            gameObject.transform2d.translation += gameObject.acceleration * deltaTime;
//            auto collisions = resolveCollisionWithOtherParticles(gameObject, gameObjects, deltaTime);
//            std::cout << "Collisions: " << collisions << std::endl;


//            resolveCollisionWithOuterWalls(gameObject, deltaTime);


            glm::vec2 position = gameObject.transform2d.translation;
            glm::vec2 last_position = gameObject.last_position;
            const glm::vec2 last_update_move = position - last_position;
            const glm::vec2 new_position = position + last_update_move +
                                           (gameObject.acceleration - last_update_move * 40.0f) *
                                           (deltaTime * deltaTime);
            gameObject.last_position = position;
            gameObject.transform2d.translation = new_position;
            gameObject.acceleration = {0.0f, 0.0f};
        }

    }

    void PhysicsSystem::resolveCollisionWithOuterWalls(RocketGameObject &gameObject, float deltaTime) {
        float radius = gameObject.radius;
        float margin = 0.001f;
        glm::vec2 center = gameObject.transform2d.translation;

        if (glm::abs(center.x) + radius >= 1.0f - margin) {
            float distance = glm::abs(gameObject.transform2d.translation.x) + radius - 1.0f +margin;
            if (center.x < 0.0f) {
                gameObject.transform2d.translation.x += distance;
            } else {
                gameObject.transform2d.translation.x -= distance;
            }
        }
        if (glm::abs(center.y) + radius >= 1.0f - margin) {
            float distance = glm::abs(gameObject.transform2d.translation.y) + radius - 1.0f +margin;
            if (center.y < 0.0f) {
                gameObject.transform2d.translation.y += distance;
            } else {
                gameObject.transform2d.translation.y -= distance;
            }
        }

    }

    uint32_t PhysicsSystem::resolveCollisionWithOtherParticles(RocketGameObject &gameObject,
                                                           std::vector<RocketGameObject> &gameObjects,
                                                           float deltaTime) {
        uint32_t collisions = 0;
        for (auto &particle: gameObjects) {

            collisions++;
            constexpr float response_coef = 1.1f;
            constexpr float eps = 0.0001f;
            const glm::vec2 o2_o1 = gameObject.transform2d.translation - particle.transform2d.translation;
            const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
            const float dinstace_minus_radius = dist2 - glm::pow(gameObject.radius + particle.radius, 2);
            if (dinstace_minus_radius < 0.0f && dist2 > eps) {
                const float dist = sqrt(dist2);
                // Radius are all equal to 1.0f
                const float delta = response_coef * 0.5f * (gameObject.radius + particle.radius - dist);
                const glm::vec2 col_vec = (o2_o1 / dist) * delta;
                    gameObject.transform2d.translation += col_vec;
                    particle.transform2d.translation -= col_vec;
            }
        }
        return collisions;

    }


    DebugInfo PhysicsSystem::getDebugInfo() {
        debugInfo.averageCellObjectCount = grid.getAverageCellObjectCount();
        debugInfo.filledCellCount = grid.getFilledCellCount();
        return debugInfo;
    }
}
