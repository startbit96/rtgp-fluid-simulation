#pragma once

#include <string>
#include <vector>

#include "scene_information.h"

#define SCENE_MODEL_DIR_RELATIVE_PATH   "../models/"
#define SCENE_CONFIG_FILE               "scene_config.json"


class Scene_Handler {
    private:

    public:
        Scene_Handler();

        int current_scene_id;
        int next_scene_id;

        std::vector<Scene_Information> available_scenes;

        bool load_scene_informations (std::string filepath_json_file = 
            std::string(SCENE_MODEL_DIR_RELATIVE_PATH) + std::string(SCENE_CONFIG_FILE));
        void register_new_scene (   std::string description, 
                                    std::string filepath_simulation_space,
                                    std::vector<std::string> filepath_fluid,
                                    std::vector<std::string> filepath_obstacles);
        bool delete_scene (int scene_id);
        void delete_all_scenes ();
        bool load_scene ();
        void print_information ();
};