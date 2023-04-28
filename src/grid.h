//
// Created by mario on 4/20/23.
//

#include <vector>
#include "rocket_game_object.hpp"

#ifndef ROCKET_GRID_H
#define ROCKET_GRID_H

#endif //ROCKET_GRID_H

namespace rocket{

        class Grid {
            enum CellType{
                AIR_CELL, SOLID_CELL, FLUID_CELL
            };

        public:

            struct Cell{
//                float velocity_x;
//                float velocity_y;
//                float velocity_dx;
//                float velocity_dy;
//                float prev_velocity_x;
//                float prev_velocity_y;
                std::vector<uint32_t> objects;
                std::vector<uint32_t> resolvedCells;
//                CellType type;
//                float p;
//                float s;

            };
            Grid(){
                this->grid_width = 1;
                this->grid_height = 1;
                grid = std::vector<Cell>(grid_height * grid_width);
                for(int i = 0; i < grid.size(); i++){
                    grid[i].objects = std::vector<uint32_t>();
                    grid[i].resolvedCells = std::vector<uint32_t>();
                }
            }
            Grid(int width, int height, std::vector<RocketGameObject> &gameObjects){
                this->grid_width = width;
                this->grid_height = height;
                this->maxParticles = gameObjects.size();
                grid = std::vector<Cell>(grid_height * grid_width);
                for(int i = 0; i < grid.size(); i++){
                    grid[i].objects = std::vector<uint32_t>();
                    grid[i].resolvedCells = std::vector<uint32_t>();
                }
            };
            ~Grid() {};

//            Grid(const Grid&) = delete;
//            void operator=(const Grid&) = delete;

        void updateGrid(std::vector<RocketGameObject> &gameObjects);
        uint32_t getContainedCell(RocketGameObject &gameObject) ;
        uint32_t resolveCollisions(std::vector<RocketGameObject> &gameObjects);
        uint32_t resolveCollisionsBetweenTwoCells(std::vector<RocketGameObject> &gameObjects, int cell1, int cell2);
        uint32_t resolveCollisionInCell(std::vector<RocketGameObject> &gameObjects, int cell);
        uint32_t resolveCollisionsWithWalls(std::vector<RocketGameObject> &gameObjects);
        uint32_t resolveCollisionsBetweenCellAndWalls(std::vector<RocketGameObject> &gameObjects, int cell, int direction); // 0 - up, 1 - right, 2 - down, 3 - left
        float getAverageCellObjectCount();
        uint32_t getFilledCellCount();
        void clearResolvedCells();
        void solveIncompressibility(int numIters, float dt,float overRelaxation, bool compensateDrift = true);
        void updateParticleDensity();
        void updatePositions(std::vector<RocketGameObject> &gameObjects);


        void transferVelocities(std::vector<RocketGameObject>& gameObjects,bool toGrid,float flipRatio);
        void printGrid();
        std::vector<Cell> getGrid();
        int getGridWidth(){
            return grid_width;
        }
        int getGridHeight(){return grid_height;};
        private:
            int grid_width;
            int grid_height;
//            std::vector<std::vector<uint32_t>> grid;
//            std::vector<std::vector<uint32_t>> resolvedCells;
            std::vector<Cell> grid;
//            void updateGrid(std::vector<RocketGameObject> &gameObjects);

        float density = 1000.f;
        float tankWidth = 2.f;
        float tankHeight = 2.f;
        float r = 0.010f;
        float fNumX =grid_width;
        float fNumY = grid_height;
        int h = 2 / (grid_height * grid_width);
        float fInvSpacing = 1.0 / h;
        int fNumCells = fNumX * fNumY;

        std::vector<float> u = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> v = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> du = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> dv = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> prevU = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> prevV = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> p = std::vector<float>(fNumCells); // array of x velocities
        std::vector<float> s = std::vector<float>(fNumCells); // array of x velocities
        std::vector<CellType> cellType = std::vector<CellType>(fNumCells); // array of x velocities
        std::vector<float> cellColor = std::vector<float>(3* fNumCells); // array of x velocities

        // particles

        int maxParticles = 1000;

        std::vector<float> particlePos = std::vector<float>(2 * maxParticles);
        std::vector<float> particleColor = std::vector<float>(3 * maxParticles);
//        for(int i = 0; i < maxParticles; i++)
//            particleColor[3 * i + 2] = 1.0;


        std::vector<float> particleVel = std::vector<float>(2 * maxParticles);
        std::vector<float> particleDensity = std::vector<float> (fNumCells);
        float particleRestDensity = 0.0;

        float particleRadius;
        float pInvSpacing = 1.0 / (2.2 * particleRadius);
        float pNumX = std::floor(grid_width * pInvSpacing) + 1;
        float pNumY = std::floor(grid_height * pInvSpacing) + 1;
        int pNumCells = pNumX * pNumY;

        std::vector<int> numCellParticles = std::vector<int>(pNumCells);
        std::vector<int> firstCellParticle = std::vector<int>(pNumCells + 1);
        std::vector<int> cellParticleIds = std::vector<int>(maxParticles);

        int numParticles = 0;
        };
}