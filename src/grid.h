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
        public:
            Grid(){
                this->grid_width = 1;
                this->grid_height = 1;
                grid = std::vector<std::vector<uint32_t>>(grid_height * grid_width, std::vector<uint32_t>());
                resolvedCells = std::vector<std::vector<uint32_t>>(grid_height * grid_width, std::vector<uint32_t>());
            }
            Grid(int width, int height){
                this->grid_width = width;
                this->grid_height = height;
                grid = std::vector<std::vector<uint32_t>>(grid_height * grid_width, std::vector<uint32_t>());
                resolvedCells = std::vector<std::vector<uint32_t>>(grid_height * grid_width, std::vector<uint32_t>());

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
        void printGrid();
        private:
            int grid_width;
            int grid_height;
            std::vector<std::vector<uint32_t>> grid;
            std::vector<std::vector<uint32_t>> resolvedCells;

//            void updateGrid(std::vector<RocketGameObject> &gameObjects);
        };
}