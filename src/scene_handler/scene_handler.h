#pragma once

#include <string>
#include <vector>

#include "scene_information.h"

#define SCENE_MODEL_DIR_RELATIVE_PATH   "../models/"
#define SCENE_CONFIG_FILE               "scene_config.json"


class Scene_Handler 
{
    private:

    public:
        int current_scene_id;
        int next_scene_id;
        std::vector<Scene_Information> available_scenes;
        Particle *particles;
        unsigned int *particle_indices;
        unsigned int number_of_particles;
        unsigned int vertex_array_object;
        unsigned int vertex_buffer_object;
        unsigned int index_buffer_object;

        Scene_Handler();

        void register_new_scene (   std::string description,
                                    Cuboid&& simulation_space,
                                    std::vector<Cuboid> fluid_starting_positions);
        bool delete_scene (int scene_id);
        void delete_all_scenes ();
        bool load_scene ();
        void calculate_initial_particle_positions ();
        Cuboid* get_pointer_to_simulation_space ();
        std::vector<Cuboid>* get_pointer_to_fluid_starting_positions ();
        glm::vec3 get_current_point_of_interest ();
        void print_information ();
};