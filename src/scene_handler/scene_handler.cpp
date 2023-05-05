#include "scene_handler.h"

#include <iostream>


Scene_Handler::Scene_Handler()
{
    this->current_scene_id = -1;
    this->next_scene_id = -1;
}

void Scene_Handler::register_new_scene (std::string scene_description, std::vector<std::string> filepath_obstacles, std::vector<std::string> filepath_fluid)
{
    this->available_scenes.push_back(
        Scene_Information(
            scene_description,
            filepath_obstacles,
            filepath_fluid
        )
    );
}

bool Scene_Handler::delete_scene (int scene_id) 
{
    if (scene_id > this->available_scenes.size() - 1) {
        return false;
    }
    this->available_scenes.erase(this->available_scenes.begin() + scene_id);
    return true;
}

void Scene_Handler::delete_all_scenes () 
{
    this->available_scenes.clear();
}

bool Scene_Handler::load_scene ()
{
    std::cout << "Loading scene with scene_id " << this->next_scene_id << "..." << std::endl;
    if (this->next_scene_id < 0) {
        std::cout << "scene_id needs to be greater than or equal to zero." << std::endl;
        return false;
    }
    if (this->next_scene_id > this->available_scenes.size() - 1) {
        std::cout << "scene_id " << this->next_scene_id << " is not registered." << std::endl; 
        return false;
    }
    this->current_scene_id = this->next_scene_id;
    return true;
}

void Scene_Handler::print_information () 
{
    
}