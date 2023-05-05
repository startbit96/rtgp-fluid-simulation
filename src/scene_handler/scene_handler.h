#pragma once

#include <string>
#include <vector>
#include "scene_information.h"

class Scene_Handler {
    private:

    public:
        Scene_Handler();

        int current_scene_id;
        int next_scene_id;

        std::vector<Scene_Information> available_scenes;

        void register_new_scene (   std::string scene_description, 
                                    std::vector<std::string> filepath_obstacles, 
                                    std::vector<std::string> filepath_fluid);
        bool delete_scene (int scene_id);
        void delete_all_scenes ();
        bool load_scene ();
        void print_information ();
};