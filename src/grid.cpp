//
// Created by mario on 4/20/23.
//
#include <iostream>
#include <algorithm>
#include <vector>
#include "grid.h"
namespace  rocket {
    void Grid::updateGrid(std::vector<RocketGameObject> &gameObjects){
        for (auto &gameObject: gameObjects) {
            uint32_t contained_cell = getContainedCell(gameObject);
            if(gameObject.gridPosition != -1 && gameObject.gridPosition != contained_cell){
                grid[gameObject.gridPosition].erase(std::remove( grid[gameObject.gridPosition].begin(),  grid[gameObject.gridPosition].end(), gameObject.getId()),  grid[gameObject.gridPosition].end());
                grid[contained_cell].push_back(gameObject.getId());
            } else if(gameObject.gridPosition == -1){
                grid[contained_cell].push_back(gameObject.getId());
            }
            gameObject.gridPosition = contained_cell;
        }
    }

    uint32_t Grid::getContainedCell(RocketGameObject &gameObject) {

        float x = gameObject.transform2d.translation.x;
        float y = gameObject.transform2d.translation.y;

        // Calculate the row and column of the cell that the object's center is inside of
        int row = floor((x + 1.0) * (grid_width / 2.0));
        int col = floor((y + 1.0) * (grid_height / 2.0));

        // Make sure the row and column are within the valid range
        row = std::max(0, std::min(row, grid_width - 1));
        col = std::max(0, std::min(col, grid_height - 1));

        // Calculate the index of the cell in the grid vector
        int cell_index = col * grid_height + row;

        return cell_index;
    }

    uint32_t Grid::resolveCollisions(std::vector<RocketGameObject> &gameObjects) {
        uint32_t collisions = 0;
        for(int i = 0; i < grid.size(); i++){
            if(grid[i].size() > 0)
                collisions +=resolveCollisionInCell(gameObjects, i);
        }
        return collisions;
    }

    uint32_t Grid::resolveCollisionInCell(std::vector<RocketGameObject> &gameObjects, int cell) {
        uint32_t collisions = 0;
        int cellX = cell % grid_width;
        int cellY = cell / grid_width;
        for(int i = -1; i< 2; i++){
            for(int j = -1; j < 2; j++){
                if((cellX +i) >= 0 && cellX + i < grid_width && cellY + j >= 0 && cellY + j < grid_height){
                    int cell_index = (cellY + j) * grid_height + (cellX + i);
                    if(grid[cell_index].size() > 0){
                        uint32_t result = resolveCollisionsBetweenTwoCells(gameObjects, cell, cell_index);
                        if(result != -1){
                            collisions += result;
                            resolvedCells[cell].push_back(cell_index);
                            resolvedCells[cell_index].push_back(cell);
                        }


                }
}
            }
        }
        return collisions;
    }


    uint32_t Grid::resolveCollisionsBetweenTwoCells(std::vector<RocketGameObject> &gameObjects, int cell1, int cell2){
        uint32_t  collisions = 0;
        if(std::count(resolvedCells[cell1].begin(), resolvedCells[cell1].end(), cell2)){
            return -1;
        }
        if(std::count(resolvedCells[cell2].begin(), resolvedCells[cell2].end(), cell1)){
            return -1;
        }
        for(int object1_id : grid[cell1]){
            for(int object2_id : grid[cell2]){
                if(object1_id != object2_id){
                    collisions++;
                    RocketGameObject &object1 = gameObjects[object1_id];
                    RocketGameObject &object2 = gameObjects[object2_id];

                    constexpr float response_coef = 1.00f;
                    constexpr float eps = 0.0001f;
                    const glm::vec2 o2_o1 = object1.transform2d.translation - object2.transform2d.translation;
                    const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
                    const float dinstace_minus_radius = dist2 - glm::pow(object1.radius + object2.radius, 2);
                    if (dinstace_minus_radius < 0.0f && dist2 > eps) {
                        const float dist = sqrt(dist2);
                        // Radius are all equal to 1.0f
                        const float delta = response_coef * 0.5f * (object1.radius + object2.radius - dist);
                        const glm::vec2 col_vec = (o2_o1 / dist) * delta;
                        object1.transform2d.translation += col_vec;
                        object2.transform2d.translation -= col_vec;
                    }

                }
            }
        }
        return collisions;
    }

    void Grid::printGrid(){
        for(int i = 0; i < grid.size(); i++){
            std::cout << "Cell " << i << ": ";
            for(int j = 0; j < grid[i].size(); j++){
                std::cout << grid[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }


    float Grid::getAverageCellObjectCount(){
        float total = 0;
        float count = 0;
        for(int i = 0; i < grid.size(); i++){
            if(grid[i].size() > 0) {
                total += grid[i].size();
                count++;
            }
        }
        return total / count;
    }

    uint32_t Grid::getFilledCellCount(){
        uint32_t count = 0;
        for(int i = 0; i < grid.size(); i++){
            if(grid[i].size() > 0) {
                count++;
            }
        }
        return count;
    }

    uint32_t Grid::resolveCollisionsWithWalls(std::vector<RocketGameObject> &gameObjects){
        uint32_t collisions = 0;
//        collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, 0, 3); // left
//        collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, grid_width - 1, 1); // right
        for(int i =0; i< grid_width; i++){
            collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, i, 0); // top
        }
        for(int i =0; i< grid_width; i++){
            int cell_index = (grid_height - 1) * grid_height + i;
            collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 2); // bottom
        }

        for(int i =0; i< grid_height; i++){
            int cell_index = i * grid_height;
            collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 3); // left
        }
        for(int i =0; i< grid_height; i++){
            int cell_index = (i + 1) * grid_height - 1;
            collisions+= resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 1); // right
        }




        return collisions;
    }

    uint32_t Grid::resolveCollisionsBetweenCellAndWalls(std::vector<RocketGameObject> &gameObjects, int cell, int direction) { // 0 - top, 1 - right, 2 - bottom, 3 - left
        for(int object_id: grid[cell]){
            RocketGameObject& gameObject = gameObjects[object_id];

            float radius = gameObject.radius;
            float margin = 0.001f;
            glm::vec2 center = gameObject.transform2d.translation;

            if(direction == 2){
                if(center.y + radius >= 1.0f - margin){
                    float distance = center.y + radius - 1.0f + margin;
                    gameObject.transform2d.translation.y -= distance;
                }
            }
            if(direction == 1){
                if(center.x + radius >= 1.0f - margin){
                    float distance = center.x + radius - 1.0f + margin;
                    gameObject.transform2d.translation.x -= distance;
                }
            }
            if(direction == 0){
                if(center.y - radius <= -1.0f + margin){
                    float distance = center.y - radius + 1.0f - margin;
                    gameObject.transform2d.translation.y -= distance;
                }
            }

            if(direction == 3){
                if(center.x - radius <= -1.0f + margin){
                    float distance = center.x - radius + 1.0f - margin;
                    gameObject.transform2d.translation.x -= distance;
                }
            }


        }

    }


    void Grid::clearResolvedCells() {
        for (int i = 0; i < resolvedCells.size(); i++) {
            resolvedCells[i].clear();
        }
    }







}


