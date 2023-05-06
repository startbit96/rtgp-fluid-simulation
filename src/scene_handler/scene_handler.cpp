#include "scene_handler.h"

#include <iostream>
#include <fstream>


Scene_Handler::Scene_Handler()
{
    this->current_scene_id = -1;
    this->next_scene_id = -1;
}

bool Scene_Handler::load_scene_informations (std::string filepath_json_file)
{
    std::ifstream json_file(filepath_json_file);
    // Check if the file exists.
    if (json_file.good() == false) {
        std::cout << "An error occured while reading in the scene informations from the file '" <<
            filepath_json_file << "'. File not found." << std::endl;
        return false;
    }
    // Try to parse the file into the std::vector of Scene_Information.
    try {
        this->available_scenes = json::parse(json_file);
    }
    catch (...) {
        std::cout << "An error occured while reading in the scene informations from the file '" <<
            filepath_json_file << "'. Format not correct. For more informations check the repository." << std::endl;
        return false;
    }
    return true;
}

void Scene_Handler::register_new_scene (std::string description, 
                                        std::string filepath_simulation_space,
                                        std::vector<std::string> filepath_fluid,
                                        std::vector<std::string> filepath_obstacles)
{
    this->available_scenes.push_back(
        Scene_Information(
            description,
            filepath_simulation_space,
            filepath_fluid,
            filepath_obstacles
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
    std::cout << std::endl << "Available scenes:" << std::endl << std::endl;
    for (Scene_Information scene_information: this->available_scenes) {
        scene_information.print_information();
        std::cout << std::endl;
    }
}