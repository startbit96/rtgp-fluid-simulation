#include <iostream>

#include "scene_information.h"

Scene_Information::Scene_Information ()
{
    // No implementation. 
    // This one is only needed for the from_json function to work.
}

Scene_Information::Scene_Information (  std::string description, 
                                        std::string filepath_simulation_space,
                                        std::vector<std::string> filepaths_fluids,
                                        std::vector<std::string> filepaths_obstacles)
{
    this->description = description;
    this->filepath_simulation_space = filepath_simulation_space;
    this->filepaths_fluids = filepaths_fluids;
    this->filepaths_obstacles = filepaths_obstacles;
}

void Scene_Information::print_information ()
{
    std::cout << "Scene '" << description << "':" << std::endl;
    std::cout << "┣━━ Simulation Space:" << std::endl;
    std::cout << "┃   ↪ " << this->filepath_simulation_space << std::endl;
    std::cout << "┣━━ Fluid Starting Positions:" << std::endl;
    for (std::string filepath: this->filepaths_fluids) {
        std::cout << "┃   ↪ " << filepath << std::endl;
    }
    std::cout << "┗━━ Obstacles:" << std::endl;
    for (std::string filepath: this->filepaths_obstacles) {
        std::cout << "    ↪ " << filepath << std::endl;
    }
}


void from_json (const json& json_file, Scene_Information& scene_information) 
{
    json_file.at("description").get_to(scene_information.description);
    json_file.at("filepath_simulation_space").get_to(scene_information.filepath_simulation_space);
    json_file.at("filepaths_fluids").get_to(scene_information.filepaths_fluids);
    json_file.at("filepaths_obstacles").get_to(scene_information.filepaths_obstacles);
}