#pragma once

#include <string>
#include <vector>

#include "../utils/cuboid.h"

class Scene_Information 
{
    private:

    public:
        std::string description;
        Cuboid simulation_space;
        std::vector<Cuboid> fluid_starting_positions;

        Scene_Information ();
        Scene_Information ( std::string description,
                            Cuboid simulation_space,
                            std::vector<Cuboid> fluid_starting_positions);
        
        bool is_valid ();
        void print_information ();
};