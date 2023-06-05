#pragma once

#include <string>
#include <vector>

#include "scene_information.h"
#include "../utils/cuboid.h"
#include "../utils/particle_system.h"

class Simulation_Handler 
{
    private:
        void calculate_initial_particle_positions ();

    public:
        int current_scene_id;
        int next_scene_id;
        std::vector<Scene_Information> available_scenes;
        Particle_System particle_system;

        Simulation_Handler();

        // Scene handling.
        void register_new_scene (   std::string description,
                                    Cuboid&& simulation_space,
                                    std::vector<Cuboid> fluid_starting_positions);
        bool delete_scene (int scene_id);
        void delete_all_scenes ();
        bool load_scene ();
        void print_scene_information ();

        // Some functions that return pointers to the cuboids and other 
        // informations needed for rendering by the visualization handler.
        Cuboid* get_pointer_to_simulation_space ();
        std::vector<Cuboid>* get_pointer_to_fluid_starting_positions ();
        Particle_System* get_pointer_to_particle_system ();
        glm::vec3 get_current_point_of_interest ();
};