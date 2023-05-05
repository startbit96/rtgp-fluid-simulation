#include <iostream>

#include "scene_information.h"

Scene_Information::Scene_Information (std::string description, std::vector<std::string> filepath_obstacles, std::vector<std::string> filepath_fluid)
{
    this->description = description;
    this->filepath_obstacles = filepath_obstacles;
    this->filepath_fluid = filepath_fluid;
}

void Scene_Information::print_information ()
{
    std::cout << description << std::endl;
}
