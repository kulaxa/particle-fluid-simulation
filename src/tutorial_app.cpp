#include "tutorial_app.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <stdexcept>
#include <array>
#include "particle.hpp"
#include "physics_system.hpp"
#include <random>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>
#include <map>

float gravity_strength = 2.f;
glm::vec2 gravity_dir = {0.0f, gravity_strength};


void toggleGravityFlip(std::atomic<bool>& toggleFlag, int stage){
    while (toggleFlag.load()) {
        if (stage == 0) {
            gravity_dir = {gravity_strength, 0.f};
            stage = 1;
            std::this_thread::sleep_for(std::chrono::milliseconds (600));

        } else if( stage == 1){
            gravity_dir = {0.0f, gravity_strength};
            stage = 2;
            std::this_thread::sleep_for(std::chrono::seconds (2));

        }
        else if ( stage == 2){
            gravity_dir = {-gravity_strength, 0.0f};
            stage = 3;
            std::this_thread::sleep_for(std::chrono::milliseconds (600));

        }
        else if ( stage == 3){
            gravity_dir = {0.0f, gravity_strength};
            stage = 0;
            std::this_thread::sleep_for(std::chrono::seconds (2));

        }
        gravity_dir = {gravity_dir.x , gravity_dir.y};
    }
}

namespace rocket {
	static std::default_random_engine generator;
	static float CIRCLE_RADIUS = 0.010f;
    static float OBSTACLE_RADIUS = 0.2f;
    bool gravity_toggle = false;

    TutorialApp::TutorialApp()
	{
		loadGameObjects();
	}
	TutorialApp::~TutorialApp()
	{
	}
	void TutorialApp::run()
	{
        std::ofstream outputFile("results.txt");
        int frame_counter = 0;
        int particle_counter = 0;
        float average_fps_particle = 0.0f;
        std::map <int, float> particle_fps = {};
		std::cout << "Starting Tutorial App." << std::endl;
		SimpleRenderSystem simpleRenderSystem(rocketDevice, rocketRenderer.getSwapChainRenderPass());

        std::atomic<bool> toggleFlag{ false };
        std::thread toggleThread;
        int stage = 0;
		while (!rocketWindow.shouldClose()) {
			glfwPollEvents();

			// Start the Dear ImGui frame
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static int i = 1;
				static int particleCounter = 0;
                static int GRID_DIMENSION = 20;
                static float FLIP_PERCENTAGE = 0.05f;
                static float OVERRELAXATION = 2.0f;
                static int FLUID_ITERATIONS = 20;
                static int COLLISION_ITERATIONS = 1;

                ImGui::Begin("Simulation configuration and info!");


				ImGui::SliderInt("Number of particles to add", &i,1, 20);
                if(ImGui::SliderInt("Number of iterations for fluid simulation", &FLUID_ITERATIONS,1, 20)){
                    physicsSystem.setFluidIterationCount(FLUID_ITERATIONS);
                }
                if (ImGui::SliderFloat("FLIP percentage", &FLIP_PERCENTAGE, 0.0f, 1.10f)){
                    physicsSystem.setFlipRatio(FLIP_PERCENTAGE);
                }
                ImGui::SliderFloat("Overrelaxation", &OVERRELAXATION, 0.0f, 2.00f);
                if(ImGui::SliderInt("Collistion iterations", &COLLISION_ITERATIONS,0, 5)){
                    physicsSystem.setCollisionIterationCount(COLLISION_ITERATIONS);
                }


                if(ImGui::SliderInt("Grid dimension (NxN)", &GRID_DIMENSION, 1, 20)){
                    std::cout << "Grid dimension changed to " << GRID_DIMENSION << std::endl;
                    // physicsSystem.setGridDimension(GRID_DIMENSION);
                }

               if(ImGui::SliderFloat("Gravity strength", &gravity_strength, 0, 20)){
                   glm::vec2 g = glm::vec2(0.0f, gravity_strength);
                   physicsSystem.setGravity(g);
                   gravity_dir = g;
               }
               else{
                   physicsSystem.setGravity(gravity_dir);

               }
                    // physicsSystem.setGridDimension(GRID_DIMENSION);



//				if (ImGui::Button("Flip Gravity")) {
//
//					glm::vec2 g = gravity_dir;
//
//					physicsSystem.setGravity(glm::vec2(g.y, g.x));
//
//				}
                if (ImGui::Button("Toggle flipping gravity")) {

                    if (!gravity_toggle) {
                        std::cout << "Toggle thread started" << std::endl;
                        toggleFlag.store(true);
                        toggleThread = std::thread(toggleGravityFlip, std::ref(toggleFlag), stage);
                        gravity_toggle = true;

                    } else{
                        std::cout << "Toggle thread stopped" << std::endl;
                        toggleFlag.store(false);
                        toggleThread.join();
                        gravity_toggle = false;
                        gravity_dir = {0.0f, gravity_strength};

                    }
                }

                if(ImGui::Button("Enable fluid physics")){
                    physicsSystem.enableFluidPhysics();
                }
                if(ImGui::Button("Disable fluid physics")){
                    physicsSystem.disableFluidPhysics();
                }


                float mouseX = 2 * (ImGui::GetMousePos().x / WIDTH - 0.5f);
				float mouseY = 2 * (ImGui::GetMousePos().y / HEIGHT - 0.5f);

                if(!obstacleCreated){
                    createObstacle({0.0f, 0.0f});
                    obstacleCreated = true;
                    std::cout << "Obstacle created" << std::endl;

                }

				if (ImGui::IsMouseDown(0)) {
					uint32_t selectedParticle = getSelectedParticle(mouseX, mouseY, 0.0f);
					if (selectedParticle != -1) {
						gameObjects[getParticleIndex(selectedParticle)].transform2d.translation = {mouseX, mouseY};

					}
				}

                if (ImGui::IsMouseDown(1)) {
                    uint32_t selectedParticle = getSelectedParticle(mouseX, mouseY, 0.02f);
                    if(selectedParticle == -1){
                        particleCounter+=i;

                        createMultipleParticles({mouseX, mouseY}, i, CIRCLE_RADIUS);
                        std::cout << i << " Particle created" << std::endl;
                    }
                }
                float FPS = ImGui::GetIO().Framerate;
                if (particleCounter == particle_counter){
                    frame_counter++;
                    average_fps_particle += FPS;
                }
                else{
                    particle_fps[particle_counter] = average_fps_particle / frame_counter;
                    frame_counter = 0;
                    average_fps_particle = 0.0f;
                    particle_counter = particleCounter;
                }

				ImGui::Text("Number of particles = %d", particleCounter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / FPS, FPS);
				ImGui::Text("Mouse position is %.3f x, %0.3f y", mouseX, mouseY);
                ImGui::Text("Fluid simulation enabled: %s", physicsSystem.isFluidPhysicsEnabled() ? "true" : "false");


				//uint32_t selectedParticle = getSelectedParticle(mouseX, mouseY, 0.0f);
				//if (selectedParticle != -1) {
                  //  ImGui::Text("[Obstacle grid position] lastPosition %d, number of gridCells %d", gameObjects[getParticleIndex(selectedParticle)].gridPosition, gameObjects[getParticleIndex(selectedParticle)].obstacleGridPositions.size());
//					ImGui::Text("[Hover Particle Info] position (%.3f, %.3f) gridPosition %d, id %d",
//						gameObjects[getParticleIndex(selectedParticle)].velocity.x,
//						gameObjects[getParticleIndex(selectedParticle)].velocity.y,
//						gameObjects[getParticleIndex(selectedParticle)].transform2d.translation.x,
//                        gameObjects[getParticleIndex(selectedParticle)].transform2d.translation.y,
//                        gameObjects[getParticleIndex(selectedParticle)].gridPosition,
//                        gameObjects[getParticleIndex(selectedParticle)].getId()

//                    );
				//}
               // DebugInfo debugInfo = physicsSystem.getDebugInfo();
             //   ImGui::Text("Average number of particles in a cell: %f, number of occupied cells %d", debugInfo.averageCellObjectCount, debugInfo.filledCellCount /1000);
// ImGui::Text("Time to update grid %d us", debugInfo.update_grid_duration);
              //  ImGui::Text("Time to resolve collision %d us", debugInfo.resolve_collisions_duration);

           //     ImGui::Text("Time to resolve wall collisions %d us", debugInfo.resolve_collisions_with_walls_duration);
             //   ImGui::Text("Collision count: %d", debugInfo.collision_count);


                ImGui::End();
			}

			// Imgui render
			ImGui::Render();

			if (auto commandBuffer = rocketRenderer.beginFrame()) {

                rocketRenderer.beginSwapChainRenderPass(commandBuffer);
				physicsSystem.updatePhysics(1/ ImGui::GetIO().Framerate, gameObjects);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				ImDrawData* draw_data = ImGui::GetDrawData();
				ImGui_ImplVulkan_RenderDrawData(draw_data, rocketRenderer.getCurrentCommandBuffer());
				rocketRenderer.endSwapChainRenderPass(commandBuffer);
				rocketRenderer.endFrame();
			}
		}
        if (gravity_toggle) {
            toggleFlag.store(false);
            toggleThread.join();
        }

		vkDeviceWaitIdle(rocketDevice.device());
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDeviceWaitIdle(rocketDevice.device());

		vkDestroyDescriptorPool(rocketDevice.device(), rocketDevice.getDescriptorPool(), nullptr);

        // print fps

        if (outputFile.is_open()) { // Check if the file was opened successfully
            //outputFile << "Particle FPS" << std::endl;
            outputFile << "Grid Size: "<< physicsSystem.getGridDimension().x << std::endl;
            for (int i = 0; i < particle_fps.size(); i++){
                outputFile << i  << " : "<< particle_fps[i] << std::endl;
            }
            //  outputFile.close(); // Close the file
            // std::cout << "Data has been written to the file." << std::endl;
        } else {
            std::cout << "Failed to open the file." << std::endl;
        }

	}


	void TutorialApp::loadGameObjects()
	{

		auto verticies = Particle::createParticleVerticies(CIRCLE_RADIUS, {0.0f, 0.0f});
        auto obstacleVerticies = Particle::createParticleVerticies(OBSTACLE_RADIUS, {0.0f, 0.0f});

        circleModel = std::make_shared<RocketModel>(rocketDevice, verticies);
        obstacleModel = std::make_shared<RocketModel>(rocketDevice, obstacleVerticies);
		
	}

	uint32_t TutorialApp::createParticle(glm::vec2 position)
	{

		std::uniform_real_distribution<double> distribution(-0.1, 0.1);
		std::uniform_real_distribution<double> colorDistribution(0, 1);
		RocketGameObject gameObject = RocketGameObject::createGameObject();
		gameObject.model = circleModel;
		gameObject.color = { colorDistribution(generator), colorDistribution(generator), colorDistribution(generator) };
		//gameObject.total_force = { 0.f, physicsSystem.getGravity().y * gameObject.mass };
		gameObject.gravityApplied = true;
		gameObject.type = RocketGameObjectType::PARTICLE;
		gameObject.radius = CIRCLE_RADIUS;
		//gameObject.acceleration = { 0.0f, 3.f };
//		gameObject.transform2d.translation = {position.x + distribution(generator), position.y +distribution(generator)};
        gameObject.transform2d.translation = {position.x , position.y };

        gameObject.last_position= gameObject.transform2d.translation;

		//gameObject.velocity = { distribution(generator),  distribution(generator) };
		//gameObject.velocity = { 0.0f, 9.8f };
		gameObjects.push_back(std::move(gameObject));
        physicsSystem.setParticleCountGrid(gameObjects.size()-1);
		//return std::make_unique<RocketGameObject>(gameObjects.back());
		return gameObjects.size() - 1;
	}

	uint32_t TutorialApp::getSelectedParticle(float xMouse, float yMouse, float offset)
	{
		for (auto& particle : gameObjects) {
			float xPart = particle.transform2d.translation.x;
			float yPart = particle.transform2d.translation.y;

			if (xMouse > xPart - particle.radius - offset && xMouse < xPart + particle.radius + offset &&
				yMouse > yPart - particle.radius - offset && yMouse < yPart + particle.radius + offset) {
				return particle.getId();
			}
		}
		return -1;
	}

	uint32_t TutorialApp::getParticleIndex(uint32_t particleId)
	{
		uint32_t counter = 0;
		for (auto& particle : gameObjects) {
			if (particle.getId() == particleId) {
				return counter;
			}
			counter++;
		}
	}

	void TutorialApp::clearSimulation()
	{
		gameObjects.clear();
	}

    uint32_t TutorialApp::createMultipleParticles(glm::vec2 position, int count, float radius){
//        std::uniform_real_distribution<double> distribution(-0.1, 0.1);
//
//        glm::vec2 pos =  {position.x + distribution(generator), position.y +distribution(generator)};
        createParticle(position);
        for (int i = -count/2; i < count-1; i++) {
            createParticle({position.x + i*(radius * 3), position.y });
        }
        return gameObjects.size() - 1;
    }


    uint32_t TutorialApp::createObstacle(glm::vec2 position){
        std::uniform_real_distribution<double> colorDistribution(0, 1);
        RocketGameObject gameObject = RocketGameObject::createGameObject();
        gameObject.model = obstacleModel;
        gameObject.color = { colorDistribution(generator), colorDistribution(generator), colorDistribution(generator) };
        //gameObject.total_force = { 0.f, physicsSystem.getGravity().y * gameObject.mass };
        gameObject.type = RocketGameObjectType::OBSTACLE;
        gameObject.radius = OBSTACLE_RADIUS;
        //gameObject.acceleration = { 0.0f, 3.f };
//		gameObject.transform2d.translation = {position.x + distribution(generator), position.y +distribution(generator)};
        gameObject.transform2d.translation = {position.x , position.y };

        gameObject.last_position= gameObject.transform2d.translation;

        //gameObject.velocity = { distribution(generator),  distribution(generator) };
        //gameObject.velocity = { 0.0f, 9.8f };
        gameObjects.push_back(std::move(gameObject));
//        physicsSystem.setParticleCountGrid(gameObjects.size());
        //return std::make_unique<RocketGameObject>(gameObjects.back());
        return gameObjects.size() - 1;
    }





}
