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

        Scene_Handler();

        void register_new_scene (   std::string description,
                                    Cuboid&& simulation_space,
                                    std::vector<Cuboid> fluid_starting_positions);
        bool delete_scene (int scene_id);
        void delete_all_scenes ();
        bool load_scene ();
        Cuboid* get_pointer_to_simulation_space ();
        std::vector<Cuboid>* get_pointer_to_fluid_starting_positions ();
        void print_information ();
};