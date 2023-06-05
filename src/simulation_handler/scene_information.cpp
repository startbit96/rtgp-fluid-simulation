#include <iostream>

#include "scene_information.h"

Scene_Information::Scene_Information ()
{
    // No implementation. 
    // This one is only needed for the from_json function to work.
}

Scene_Information::Scene_Information (  std::string description,
                                        Cuboid simulation_space,
                                        std::vector<Cuboid> fluid_starting_positions)
{
    this->description = description;
    this->simulation_space = simulation_space;
    this->fluid_starting_positions = fluid_starting_positions;
}

bool Scene_Information::is_valid ()
{
    for (int i = 0; i < this->fluid_starting_positions.size(); i++) {
        if (this->simulation_space.contains(this->fluid_starting_positions[i]) == false) {
            return false;
        }
    }
    return true;
}

void Scene_Information::print_information ()
{
    std::cout << this->description << std::endl;
}