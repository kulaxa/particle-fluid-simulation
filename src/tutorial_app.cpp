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

namespace rocket {
	static std::default_random_engine generator;
	static float CIRCLE_RADIUS = 0.015f;
    static float OBSTACLE_RADIUS = 0.2f;
	TutorialApp::TutorialApp()
	{
		loadGameObjects();
	}
	TutorialApp::~TutorialApp()
	{
	}
	void TutorialApp::run()
	{
		std::cout << "Starting Tutorial App." << std::endl;
		SimpleRenderSystem simpleRenderSystem(rocketDevice, rocketRenderer.getSwapChainRenderPass());

		//uint32_t testBallPosition = createParticle({ 0.f, 0.f });
		//gameObjects[testBallPosition].acceleration = glm::vec2(0.0f, 2.0f);
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

				ImGui::Begin("Fps and slider!");                          // Create a window called "Hello, world!" and append into it.


				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

				ImGui::SliderInt("Number of particles to add", &i,1, 100);            // Edit 1 float using a slider from 0.0f to 1.0f

				if (ImGui::SliderFloat("Particle radius", &CIRCLE_RADIUS, 0.0f, 0.1f)) {
					loadGameObjects();
				}
					// Edit 1 float using a slider from 0.0f to 1.0f

				if (ImGui::Button("Clear Simulation")) {
					clearSimulation();
					particleCounter = 0;
				}                        // Buttons return true when clicked (most widgets return true when edited/activated)
				float mouseX = 2 * (ImGui::GetMousePos().x / WIDTH - 0.5f);
				float mouseY = 2 * (ImGui::GetMousePos().y / HEIGHT - 0.5f);

                if(!obstacleCreated){
                    createObstacle({0.0f, 0.0f});
                    std::cout << "Obstacle created" << std::endl;
                    obstacleCreated = true;
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

                        createMultipleParticles({mouseX, mouseY}, i);
                        std::cout << i << " Particle created" << std::endl;
                    }
                }
				ImGui::Text("counter = %d", particleCounter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::Text("Mouse position is %.3f x, %0.3f y", mouseX, mouseY);
				uint32_t selectedParticle = getSelectedParticle(mouseX, mouseY, 0.0f);
				if (selectedParticle != -1) {
                    ImGui::Text("[Obstacle grid position] lastPosition %d, number of gridCells %d", gameObjects[getParticleIndex(selectedParticle)].gridPosition, gameObjects[getParticleIndex(selectedParticle)].obstacleGridPositions.size());
//					ImGui::Text("[Hover Particle Info] position (%.3f, %.3f) gridPosition %d, id %d",
//						gameObjects[getParticleIndex(selectedParticle)].velocity.x,
//						gameObjects[getParticleIndex(selectedParticle)].velocity.y,
//						gameObjects[getParticleIndex(selectedParticle)].transform2d.translation.x,
//                        gameObjects[getParticleIndex(selectedParticle)].transform2d.translation.y,
//                        gameObjects[getParticleIndex(selectedParticle)].gridPosition,
//                        gameObjects[getParticleIndex(selectedParticle)].getId()

//                    );
				}
                DebugInfo debugInfo = physicsSystem.getDebugInfo();
                ImGui::Text("Average number of particles in a cell: %f, number of occupied cells %d", debugInfo.averageCellObjectCount, debugInfo.filledCellCount /1000);
                ImGui::Text("Time to update grid %d us", debugInfo.update_grid_duration);
                ImGui::Text("Time to resolve collision %d us", debugInfo.resolve_collisions_duration);

                ImGui::Text("Time to resolve wall collisions %d us", debugInfo.resolve_collisions_with_walls_duration);
                ImGui::Text("Collision count: %d", debugInfo.collision_count);


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
		vkDeviceWaitIdle(rocketDevice.device());
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDeviceWaitIdle(rocketDevice.device());

		vkDestroyDescriptorPool(rocketDevice.device(), rocketDevice.getDescriptorPool(), nullptr);


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

    uint32_t TutorialApp::createMultipleParticles(glm::vec2 position, int count){
//        std::uniform_real_distribution<double> distribution(-0.1, 0.1);
//
//        glm::vec2 pos =  {position.x + distribution(generator), position.y +distribution(generator)};
        createParticle(position);
        for (int i = 0; i < count-1; i++) {
            createParticle({position.x, position.y - i*0.05f});
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
        gameObject.radius = 0.2f;
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
