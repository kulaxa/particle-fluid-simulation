#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include "particle.hpp"
#include "grid.h"
#include "physics_system.hpp"

namespace rocket {
    struct DebugInfo{
        float averageCellObjectCount;
        uint32_t filledCellCount;
        std::chrono::microseconds update_grid_duration;
        std::chrono::microseconds resolve_collisions_duration;
        std::chrono::microseconds  resolve_collisions_with_walls_duration;
        uint32_t collision_count;
    };
	class PhysicsSystem {
	public:
		PhysicsSystem(glm::vec2 gravity, int gridDimension);
		~PhysicsSystem();

		PhysicsSystem(const PhysicsSystem&) = delete;
		void operator=(const PhysicsSystem&) = delete;

		void updatePhysics(float deltaTime, std::vector<RocketGameObject>& gameObjects);

		void resolveCollisionWithOuterWalls(RocketGameObject& gameObject, float deltaTime);

		uint32_t resolveCollisionWithOtherParticles(RocketGameObject& gameObject, std::vector<RocketGameObject>& gameObjects, float deltaTime);

        void setParticleCountGrid(int particleCount) { grid.setParticleCount(particleCount);}

        DebugInfo getDebugInfo();


		void setGravity(glm::vec2 gravity) { this->gravity = gravity; }
		glm::vec2 getGravity() { return gravity; }

        void setGridDimension(int gridDimension) { grid = Grid(gridDimension, gridDimension); }
	private:
		glm::vec2 gravity;
        Grid grid;
        DebugInfo debugInfo;
	};
}