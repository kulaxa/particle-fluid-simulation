//
// Created by mario on 4/23/23.
//

#include <functional>
#include "fluid_solver.h"

namespace rocket {
//    void FluidSolver::transferVelocityToGrid(std::vector<RocketGameObject> &gameObjects) {
//        for (Grid::Cell cell: grid.getGrid()) {
//            for (auto particle_id: cell.objects) {
//                RocketGameObject &particle = gameObjects[particle_id];
//                int h = grid.getGridHeight();
//                int w = grid.getGridWidth();
//                int grid_x_ind = particle.gridPosition % w;
//                int grid_y_ind = particle.gridPosition / h;
//                float grid_x = grid_x_ind * (w / 2.0);
//                float grid_y = grid_y_ind * (h / 2.0);
//                float delta_x = particle.transform2d.translation.x - grid_x;
//                float delta_y = particle.transform2d.translation.y - grid_y;
//                float w1 = (1 - delta_x / h) * (1 - delta_y / h);
//                float w2 = (delta_x / h) * (1 - delta_y / h);
//                float w3 = (delta_x / h) * (delta_y / h);
//                float w4 = (1 - delta_x / h) * (delta_y / h);
//            }
//        }
//    }



}