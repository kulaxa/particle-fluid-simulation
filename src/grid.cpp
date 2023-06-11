//
// Created by mario on 4/20/23.
//
#include <iostream>
#include <algorithm>
#include <vector>
#include "grid.h"
namespace  rocket {
    void Grid::updateGrid(std::vector<RocketGameObject> &gameObjects) {
        // Clear the grid
        clearGrid();
        for (auto &gameObject: gameObjects) {
            if(gameObject.type == RocketGameObjectType::OBSTACLE) {

                for(auto &contained_cell: getContainedCellsForObstacle(gameObject)) {

                    gameObject.gridPosition = contained_cell;
                    gameObject.obstacleGridPositions.push_back(contained_cell);
                    grid[contained_cell].objects.push_back(gameObject.getId());
                }
                continue;
            }
           // uint32_t contained_cell = getContainedCell(gameObject);
            std::vector<int> contained_cells = getContainedCellsForObstacle(gameObject);
            gameObject.gridPositions = contained_cells;
            for(auto &contained_cell: contained_cells) {
                gameObject.gridPosition = contained_cell;
                grid[contained_cell].objects.push_back(gameObject.getId());
            }

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

    std::vector<int> Grid::getContainedCellsForObstacle(RocketGameObject &gameObject) {
        float x = gameObject.transform2d.translation.x;
        float y = gameObject.transform2d.translation.y;
        float R = gameObject.radius;
        int gridWidth = grid_width;
        int gridHeight = grid_height;

        // create a vector to store the cells that intersect with the circle
        int xMin = std::max(0, static_cast<int>(std::floor((x - R) * gridWidth / 2 + gridWidth / 2)));
        int yMin = std::max(0, static_cast<int>(std::floor((y - R) * gridHeight / 2 + gridHeight / 2)));
        int xMax = std::min(gridWidth - 1, static_cast<int>(std::floor((x + R) * gridWidth / 2 + gridWidth / 2)));
        int yMax = std::min(gridHeight - 1, static_cast<int>(std::floor((y + R) * gridHeight / 2 + gridHeight / 2)));

        if(xMin >= gridWidth){
            xMin = gridWidth-1;
        }
        if(yMin >= gridHeight){
            yMin = gridHeight-1;
        }
        if(xMax < 0){
            xMax = 0;
        }
        if(yMax < 0){
            yMax = 0;
        }
        // Add all cells in the bounding box that are fully or partially covered by the circle
        std::vector<int> cellsContainingBall;
        for (int yIndex = yMin; yIndex <= yMax; ++yIndex) {
            for (int xIndex = xMin; xIndex <= xMax; ++xIndex) {
                    cellsContainingBall.push_back({yIndex * gridWidth + xIndex});
            }
        }

        return cellsContainingBall;
    }



    uint32_t Grid::resolveCollisions(std::vector<RocketGameObject> &gameObjects) {
        uint32_t collisions = 0;
        for (int i = 0; i < grid.size(); i++) {
            if (grid[i].objects.size() > 0)
                collisions += resolveCollisionInCell(gameObjects, i);
        }
        return collisions;
    }

    uint32_t Grid::resolveCollisionInCell(std::vector<RocketGameObject> &gameObjects, int cell) {
        uint32_t collisions = 0;
        int cellX = cell % grid_width;
        int cellY = cell / grid_width;
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                if ((cellX + i) >= 0 && cellX + i < grid_width && cellY + j >= 0 && cellY + j < grid_height) {
                    int cell_index = (cellY + j) * grid_height + (cellX + i);
                    if (grid[cell_index].objects.size() > 0) {
                        uint32_t result = resolveCollisionsBetweenTwoCells(gameObjects, cell, cell_index);
                        if (result != -1) {
                            collisions += result;
                            grid[cell].resolvedCells.push_back(cell_index);
                            grid[cell_index].resolvedCells.push_back(cell);
                        }


                    }
                }
            }
        }
        return collisions;
    }


    uint32_t Grid::resolveCollisionsBetweenTwoCells(std::vector<RocketGameObject> &gameObjects, int cell1, int cell2) {
        uint32_t collisions = 0;
        if (std::count(grid[cell1].resolvedCells.begin(), grid[cell1].resolvedCells.end(), cell2)) {
            return -1;
        }
        if (std::count(grid[cell2].resolvedCells.begin(), grid[cell2].resolvedCells.end(), cell1)) {
            return -1;
        }
        for (int i = 0; i < grid[cell1].objects.size(); i++) {
            for (int j = 0; j < grid[cell2].objects.size(); j++) {
                int object1_id = grid[cell1].objects[i];
                int object2_id = grid[cell2].objects[j];
                if (object1_id != object2_id) {
                    collisions++;
                    RocketGameObject &object1 = gameObjects[object1_id];
                    RocketGameObject &object2 = gameObjects[object2_id];

                    constexpr float response_coef = 1.00f;
                    constexpr float eps = 0.00001f;
                    const glm::vec2 o2_o1 = object1.transform2d.translation - object2.transform2d.translation;
                    const float dist2 = o2_o1.x * o2_o1.x + o2_o1.y * o2_o1.y;
                    const float dinstace_minus_radius = dist2 - glm::pow(object1.radius + object2.radius, 2);
                    if (dinstace_minus_radius < 0.0f && dist2 > eps) {

                        const float dist = sqrt(dist2);
                        const float delta = response_coef * 0.5f * (object1.radius + object2.radius - dist);
                        const glm::vec2 col_vec = (o2_o1 / dist) * delta;
                        if(object1.type == rocket::RocketGameObjectType::OBSTACLE){
                            object2.transform2d.translation -=1.f* col_vec;
                            continue;
                        }
                        if(object2.type == rocket::RocketGameObjectType::OBSTACLE){
                            object1.transform2d.translation += 1.f*col_vec;
                            continue;
                        }
                        object1.transform2d.translation += col_vec;
                        object2.transform2d.translation -= col_vec;
                    }

                }
            }
        }
        return collisions;
    }

    void Grid::clearGrid(){
        for (int i = 0; i < grid.size(); i++) {
            grid[i].objects.clear();
            //grid[i].resolvedCells.clear();
        }
    }

    void Grid::printGrid() {
        for (int i = 0; i < grid.size(); i++) {
            std::cout << "Cell " << i << ": ";
            for (int j = 0; j < grid[i].objects.size(); j++) {
                std::cout << grid[i].objects[j] << " ";
            }
            std::cout << std::endl;
        }
    }


    float Grid::getAverageCellObjectCount() {
        float total = 0;
        float count = 0;
        for (int i = 0; i < grid.size(); i++) {
            if (grid[i].objects.size() > 0) {
                total += grid[i].objects.size();
                count++;
            }
        }
        return total / count;
    }

    uint32_t Grid::getFilledCellCount() {
        uint32_t count = 0;
        for (int i = 0; i < grid.size(); i++) {
            if (grid[i].objects.size() > 0) {
                count++;
            }
        }
        return count;
    }

    uint32_t Grid::resolveCollisionsWithWalls(std::vector<RocketGameObject> &gameObjects) {

        for (int i = 0; i < grid_width; i++) {
            resolveCollisionsBetweenCellAndWalls(gameObjects, i, 0); // top
        }
        for (int i = 0; i < grid_width; i++) {
            int cell_index = (grid_height - 1) * grid_height + i;
            resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 2); // bottom
        }

        for (int i = 0; i < grid_height; i++) {
            int cell_index = i * grid_height;
            resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 3); // left
        }
        for (int i = 0; i < grid_height; i++) {
            int cell_index = (i + 1) * grid_height - 1;
            resolveCollisionsBetweenCellAndWalls(gameObjects, cell_index, 1); // right
        }

    }

    uint32_t Grid::resolveCollisionsBetweenCellAndWalls(std::vector<RocketGameObject> &gameObjects, int cell,
                                                        int direction) { // 0 - top, 1 - right, 2 - bottom, 3 - left
        for (int object_id: grid[cell].objects) {
            RocketGameObject &gameObject = gameObjects[object_id];
            float radius = gameObject.radius;
            float margin = 0.001f;
            glm::vec2 center = gameObject.transform2d.translation;
            if (direction == 2) {
                if (center.y + radius >= 1.0f - margin) {
                    float distance = center.y + radius - 1.0f + margin;
                    gameObject.transform2d.translation.y -= distance;
                }
            }
            if (direction == 1) {
                if (center.x + radius >= 1.0f - margin) {
                    float distance = center.x + radius - 1.0f + margin;
                    gameObject.transform2d.translation.x -= distance;
                }
            }
            if (direction == 0) {
                if (center.y - radius <= -1.0f + margin) {
                    float distance = center.y - radius + 1.0f - margin;
                    gameObject.transform2d.translation.y -= distance;
                }
            }
            if (direction == 3) {
                if (center.x - radius <= -1.0f + margin) {
                    float distance = center.x - radius + 1.0f - margin;
                    gameObject.transform2d.translation.x -= distance;
                }
            }


        }

    }


    void Grid::clearResolvedCells() {
        for (int i = 0; i < grid.size(); i++) {
            grid[i].resolvedCells.clear();
        }

    }


    void Grid::transferVelocities( bool toGrid, float flipRatio) {
        auto n = fNumY;
        auto h1 = fInvSpacing;
        auto h2 = 0.5 * h;

        if (toGrid) {

            prevU.clear();
            prevV.clear();

            std::copy(u.begin(), u.end(), std::back_inserter(prevU));
            std::copy(v.begin(), v.end(), std::back_inserter(prevV));

            du = std::vector<float>(fNumCells, 0.0);
            dv = std::vector<float>(fNumCells, 0.0);

            u = std::vector<float>(fNumCells, 0.0);
            v = std::vector<float>(fNumCells, 0.0);

            for (int i = 0; i < fNumCells; i++)
                cellType[i] = s[i] == 0.0 ? SOLID_CELL : AIR_CELL;

            for (int i = 0; i < numParticles; i++) {
                auto x = particlePos[2 * i];
                auto y = particlePos[2 * i + 1];
                auto xi = std::clamp<int>(std::floor(x * h1), 0, fNumX - 1);
                auto yi = std::clamp<int>(std::floor(y * h1), 0, fNumY - 1);
                auto cellNr = xi * n + yi;
                if (cellType[cellNr] == AIR_CELL)
                    cellType[cellNr] = FLUID_CELL;
            }
        }

        for (auto component = 0; component < 2; component++) {

            auto dx = component == 0 ? 0.0 : h2;
            auto dy = component == 0 ? h2 : 0.0;

            auto f = component == 0 ? u : v;
            auto prevF = component == 0 ? prevU : prevV;
            auto d = component == 0 ? du : dv;

            for (auto i = 0; i < numParticles; i++) {
                float x = particlePos[2 * i];
                float y = particlePos[2 * i + 1];

                x = std::clamp<float>(x, h, (fNumX - 1) * h);
                y = std::clamp<float>(y, h, (fNumY - 1) * h);


                auto x0 = std::min<float>(std::floor((x - dx) * h1), fNumX - 2);
                auto tx = ((x - dx) - x0 * h) * h1;
                auto x1 = std::min<float>(x0 + 1, fNumX - 2);

                auto y0 = std::min<float>(std::floor((y - dy) * h1), fNumY - 2);
                auto ty = ((y - dy) - y0 * h) * h1;
                auto y1 = std::min(y0 + 1, fNumY - 2);

                auto sx = 1.0 - tx;
                auto sy = 1.0 - ty;

                auto d0 = sx * sy;
                auto d1 = tx * sy;
                auto d2 = tx * ty;
                auto d3 = sx * ty;


                auto nr0 = x0 * n + y0;
                auto nr1 = x1 * n + y0;
                auto nr2 = x1 * n + y1;
                auto nr3 = x0 * n + y1;

                if (toGrid) {
                    auto pv = particleVel[2 * i + component];
                    f[nr0] += pv * d0;
                    d[nr0] += d0;
                    f[nr1] += pv * d1;
                    d[nr1] += d1;
                    f[nr2] += pv * d2;
                    d[nr2] += d2;
                    f[nr3] += pv * d3;
                    d[nr3] += d3;
                } else {
                    auto offset = component == 0 ? n : 1;
                    auto valid0 = cellType[nr0] != AIR_CELL || cellType[nr0 - offset] != AIR_CELL ? 1.0 : 0.0;
                    auto valid1 = cellType[nr1] != AIR_CELL || cellType[nr1 - offset] != AIR_CELL ? 1.0 : 0.0;
                    auto valid2 = cellType[nr2] != AIR_CELL || cellType[nr2 - offset] != AIR_CELL ? 1.0 : 0.0;
                    auto valid3 = cellType[nr3] != AIR_CELL || cellType[nr3 - offset] != AIR_CELL ? 1.0 : 0.0;

                    auto v = particleVel[2 * i + component];
                    auto d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;

                    if (d > 0.0) {

                        auto picV = (valid0 * d0 * f[nr0] + valid1 * d1 * f[nr1] + valid2 * d2 * f[nr2] +
                                     valid3 * d3 * f[nr3]) / d;
                        auto corr = (valid0 * d0 * (f[nr0] - prevF[nr0]) + valid1 * d1 * (f[nr1] - prevF[nr1])
                                     + valid2 * d2 * (f[nr2] - prevF[nr2]) + valid3 * d3 * (f[nr3] - prevF[nr3])) / d;
                        auto flipV = v + corr;

                        particleVel[2 * i + component] = (1.0 - flipRatio) * picV + flipRatio * flipV;
                    }
                }
            }

            if (toGrid) {
                for (auto i = 0; i < f.size(); i++) {
                    if (d[i] > 0.0)
                        f[i] /= d[i];
                }

                // restore solid cells
                for (auto i = 0; i < fNumX; i++) {
                    for (auto j = 0; j < fNumY; j++) {
                        auto solid = cellType[i * n + j] == SOLID_CELL;
                        if (solid || (i > 0 && cellType[(i - 1) * n + j] == SOLID_CELL))
                            u[i * n + j] = prevU[i * n + j];
                        if (solid || (j > 0 && cellType[i * n + j - 1] == SOLID_CELL))
                            v[i * n + j] = prevV[i * n + j];
                    }
                }
            }
    }

    }


    void Grid::solveIncompressibility(int numIters, float dt, float overRelaxation, bool compensateDrift) {

        p = std::vector<float>(fNumCells, 0.0);

        prevU.clear();
        std::copy(u.begin(), u.end(), std::back_inserter(prevU));

        prevV.clear();
//        prevU.set(u);
        std::copy(v.begin(), v.end(), std::back_inserter(prevV));

//        prevV.set(v);

        auto n = fNumY;
        auto cp = density * h / dt;

        for (auto iter = 0; iter < numIters; iter++) {

            for (auto i = 1; i < fNumX - 1; i++) {
                for (auto j = 1; j < fNumY - 1; j++) {
                    if (cellType[i * n + j] != FLUID_CELL)
                        continue;
                    auto center = i * n + j;
                    auto left = (i - 1) * n + j;
                    auto right = (i + 1) * n + j;
                    auto bottom = i * n + j - 1;
                    auto top = i * n + j + 1;
                    auto sx0 = s[left];
                    auto sx1 = s[right];
                    auto sy0 = s[bottom];
                    auto sy1 = s[top];
                    auto s = sx0 + sx1 + sy0 + sy1;
                    if (s == 0.0)
                        continue;
                    auto div = u[right] - u[center] + v[top] - v[center];
                    if (particleRestDensity > 0.0 && compensateDrift) {
                        auto k = 1.0;
                        auto compression = particleDensity[i * n + j] - particleRestDensity;
                        if (compression > 0.0)
                            div = div - k * compression;
                    }
                    auto p_t = -div / s;
                    p_t *= overRelaxation;
                    p[center] += cp * p_t;
                    u[center] -= sx0 * p_t;
                    u[right] += sx1 * p_t;
                    v[center] -= sy0 * p_t;
                    v[top] += sy1 * p_t;
                }
            }
        }
    }


    void Grid::updateParticleDensity() {
        auto n = fNumY;
        auto h1 = fInvSpacing;
        auto h2 = 0.5 * h;

        auto d = particleDensity;

//        d.fill(0.0);
        particleDensity = std::vector<float>(fNumCells, 0.0);

        for (auto i = 0; i < numParticles; i++) {
            auto x = particlePos[2 * i];
            auto y = particlePos[2 * i + 1];

            x = std::clamp<float>(x, h, (fNumX - 1) * h);
            y = std::clamp<float>(y, h, (fNumY - 1) * h);

            auto x0 = std::floor((x - h2) * h1);
            auto tx = ((x - h2) - x0 * h) * h1;
            auto x1 = std::min<float>(x0 + 1, fNumX - 2);

            auto y0 = std::floor((y - h2) * h1);
            auto ty = ((y - h2) - y0 * h) * h1;
            auto y1 = std::min<float>(y0 + 1, fNumY - 2);

            auto sx = 1.0 - tx;
            auto sy = 1.0 - ty;

            if (x0 < fNumX && y0 < fNumY) d[x0 * n + y0] += sx * sy;
            if (x1 < fNumX && y0 < fNumY) d[x1 * n + y0] += tx * sy;
            if (x1 < fNumX && y1 < fNumY) d[x1 * n + y1] += tx * ty;
            if (x0 < fNumX && y1 < fNumY) d[x0 * n + y1] += sx * ty;
        }

        if (particleRestDensity == 0.0) {
            auto sum = 0.0;
            auto numFluidCells = 0;

            for (auto i = 0; i < fNumCells; i++) {
                if (cellType[i] == FLUID_CELL) {
                    sum += d[i];
                    numFluidCells++;
                }
            }

            if (numFluidCells > 0)
                particleRestDensity = sum / numFluidCells;
        }


    }


    void Grid::updatePositionsFromGridToObject(std::vector<RocketGameObject> &gameObjects){
        for(int i =0 ,  particleIndex = 0; i< gameObjects.size(); i++,  particleIndex++){
            if(gameObjects[i].type == rocket::RocketGameObjectType::OBSTACLE) {
                particleIndex--;
                continue;
            }
            gameObjects[i].transform2d.translation.x = particlePos[2*particleIndex];
            gameObjects[i].transform2d.translation.y = particlePos[2*particleIndex+1];
        }
    }

    void Grid::updatePositionsFromObjectToGrid(std::vector<RocketGameObject> &gameObjects){
        if(maxParticles != gameObjects.size() - numOfObstacles){
            maxParticles = gameObjects.size() - numOfObstacles;
            particlePos = std::vector<float>(2 * maxParticles);
        }

        for(int i =0 , particleIndex = 0; i< gameObjects.size(); i++ , particleIndex++){
            if(gameObjects[i].type == rocket::RocketGameObjectType::OBSTACLE) {
                particleIndex--;
                continue;
            }
            particlePos[2*particleIndex] = gameObjects[i].transform2d.translation.x;
            particlePos[2*particleIndex+1] = gameObjects[i].transform2d.translation.y;
        }
    }


    void Grid::updateObstacleCellsToSolid(std::vector<RocketGameObject> &gameObjects) {
        for (auto &gameObject: gameObjects) {
            if (gameObject.type != rocket::RocketGameObjectType::OBSTACLE) {
                std::vector<int> obstacleCells = getContainedCellsForObstacle(gameObject);

                for (int i = 0; i < obstacleCells.size(); i++) {
                    cellType[obstacleCells[i]] = CellType::SOLID_CELL;
                }

            }


        }
    }

    void Grid::updateObstacleCellsToAir(std::vector<RocketGameObject> & gameObjects){
        for(auto & gameObject : gameObjects){
            if(gameObject.type != rocket::RocketGameObjectType::OBSTACLE) {


                for (int i = 0; i < cellType.size(); i++) {
                    cellType[i] = CellType::AIR_CELL;
                }
            }
            }
    }





}