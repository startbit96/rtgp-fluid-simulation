#pragma once

#include <string>
#include <vector>

class Scene_Information {
    private:

    public:
        std::string description;
        std::vector<std::string> filepath_obstacles;
        std::vector<std::string> filepath_fluid;

        Scene_Information (
            std::string description, 
            std::vector<std::string> filepath_obstacles, 
            std::vector<std::string> filepath_fluid);

        void print_information ();
};