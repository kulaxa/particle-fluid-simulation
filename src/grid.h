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
                this->maxParticles = 1;
                fNumX =grid_width;
                fNumY = grid_height;
                grid = std::vector<Cell>(grid_height * grid_width);
                h = 2 / (grid_height * grid_width);
                fInvSpacing = 1.0 / h;
                fNumCells = fNumX * fNumY;


              u = std::vector<float>(fNumCells); // array of x velocities
                v = std::vector<float>(fNumCells); // array of x velocities
                 du = std::vector<float>(fNumCells); // array of x velocities
                 dv = std::vector<float>(fNumCells); // array of x velocities
                 prevU = std::vector<float>(fNumCells); // array of x velocities
                 prevV = std::vector<float>(fNumCells); // array of x velocities
                 p = std::vector<float>(fNumCells); // array of x velocities
           s = std::vector<float>(fNumCells); // array of x velocities
                cellType = std::vector<CellType>(fNumCells); // array of x velocities
               cellColor = std::vector<float>(3* fNumCells); // array of x velocities

                // particles

               maxParticles = 1000;

                 particlePos = std::vector<float>(2 * maxParticles);
                 particleColor = std::vector<float>(3 * maxParticles);
//        for(int i = 0; i < maxParticles; i++)
//            particleColor[3 * i + 2] = 1.0;


               particleVel = std::vector<float>(2 * maxParticles);
                particleDensity = std::vector<float> (fNumCells);
                 particleRestDensity = 0.0;

                 pInvSpacing = 1.0 / (2.2 * particleRadius);
                 pNumX = std::floor(grid_width * pInvSpacing) + 1;
                 pNumY = std::floor(grid_height * pInvSpacing) + 1;
                 pNumCells = pNumX * pNumY;

              numCellParticles = std::vector<int>(pNumCells);
                firstCellParticle = std::vector<int>(pNumCells + 1);
                cellParticleIds = std::vector<int>(maxParticles);

                 numParticles = 0;
                for(int i = 0; i < grid.size(); i++){
                    grid[i].objects = std::vector<uint32_t>();
                    grid[i].resolvedCells = std::vector<uint32_t>();
                }
            }
            Grid(int width, int height){
                this->grid_width = width;
                this->grid_height = height;
                this->maxParticles = 1;
                fNumX =grid_width;
                fNumY = grid_height;
                grid = std::vector<Cell>(grid_height * grid_width);
                h = 2.f / (grid_height * grid_width);
                fInvSpacing = 1.0 / h;
                fNumCells = fNumX * fNumY;


                u = std::vector<float>(fNumCells); // array of x velocities
                v = std::vector<float>(fNumCells); // array of x velocities
                du = std::vector<float>(fNumCells); // array of x velocities
                dv = std::vector<float>(fNumCells); // array of x velocities
                prevU = std::vector<float>(fNumCells); // array of x velocities
                prevV = std::vector<float>(fNumCells); // array of x velocities
                p = std::vector<float>(fNumCells); // array of x velocities
                s = std::vector<float>(fNumCells); // array of x velocities
                cellType = std::vector<CellType>(fNumCells); // array of x velocities
                cellColor = std::vector<float>(3* fNumCells); // array of x velocities

                // particles

                maxParticles = 1000;

                particlePos = std::vector<float>(2 * maxParticles);
                particleColor = std::vector<float>(3 * maxParticles);
//        for(int i = 0; i < maxParticles; i++)
//            particleColor[3 * i + 2] = 1.0;


                particleVel = std::vector<float>(2 * maxParticles);
                particleDensity = std::vector<float> (fNumCells);
                particleRestDensity = 0.0;

                pInvSpacing = 1.0 / (2.2 * particleRadius);
                pNumX = std::floor(grid_width * pInvSpacing) + 1;
                pNumY = std::floor(grid_height * pInvSpacing) + 1;
                pNumCells = pNumX * pNumY;

                numCellParticles = std::vector<int>(pNumCells);
                firstCellParticle = std::vector<int>(pNumCells + 1);
                cellParticleIds = std::vector<int>(maxParticles);

                numParticles = 0;
                for(int i = 0; i < grid.size(); i++){
                    grid[i].objects = std::vector<uint32_t>();
                    grid[i].resolvedCells = std::vector<uint32_t>();
                    cellType[i] = CellType::AIR_CELL;
                }


                for (int i = 0; i < grid_width; i++) {
                    int cell_index = i;
                    cellType[cell_index] = CellType::SOLID_CELL;

                }
                for (int i = 0; i < grid_width; i++) {
                    int cell_index = (grid_height - 1) * grid_height + i;
                    cellType[cell_index] = CellType::SOLID_CELL;

                }

                for (int i = 0; i < grid_height; i++) {
                    int cell_index = i * grid_height;
                    cellType[cell_index] = CellType::SOLID_CELL;

                }
                for (int i = 0; i < grid_height; i++) {
                    int cell_index = (i + 1) * grid_height - 1;
                    cellType[cell_index] = CellType::SOLID_CELL;

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
        void updateObstacleCellsToSolid(std::vector<RocketGameObject> &gameObjects);
        void updateObstacleCellsToAir(std::vector<RocketGameObject> &gameObjects);

        void updateParticleDensity();
        void updatePositionsFromGridToObject(std::vector<RocketGameObject> &gameObjects);
        void updatePositionsFromObjectToGrid(std::vector<RocketGameObject> &gameObjects);
        void setParticleCount(int count){ numParticles = count; };
        std::vector<int> getContainedCellsForObstacle(RocketGameObject &gameObject);



            void transferVelocities(bool toGrid,float flipRatio);
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
        float r = 0.015f;
        float fNumX;
        float fNumY ;
        float h ;
        float fInvSpacing ;
        int fNumCells;

        std::vector<float> u; // array of x velocities
        std::vector<float> v ; // array of x velocities
        std::vector<float> du ; // array of x velocities
        std::vector<float> dv ; // array of x velocities
        std::vector<float> prevU ; // array of x velocities
        std::vector<float> prevV ; // array of x velocities
        std::vector<float> p ; // array of x velocities
        std::vector<float> s ; // array of x velocities
        std::vector<CellType> cellType; // array of x velocities
        std::vector<float> cellColor ; // array of x velocities

        // particles

        int maxParticles = 1000;

        std::vector<float> particlePos ;
        std::vector<float> particleColor ;
//        for(int i = 0; i < maxParticles; i++)
//            particleColor[3 * i + 2] = 1.0;


        std::vector<float> particleVel ;
        std::vector<float> particleDensity ;
        float particleRestDensity ;

        float particleRadius = 0.01;
        float pInvSpacing ;
        float pNumX ;
        float pNumY;
        int pNumCells;

        std::vector<int> numCellParticles ;
        std::vector<int> firstCellParticle ;
        std::vector<int> cellParticleIds ;

        int numParticles = 0;
        int numOfObstacles = 1 ;// hardcoded for now
        };
}