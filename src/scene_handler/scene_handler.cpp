#include "scene_handler.h"

#include <iostream>
#include <fstream>


Scene_Handler::Scene_Handler()
{
    this->current_scene_id = -1;
    this->next_scene_id = -1;
    this->particles = NULL;
    this->number_of_particles = 1000;
}

void Scene_Handler::register_new_scene (std::string description,
                                        Cuboid&& simulation_space,
                                        std::vector<Cuboid> fluid_starting_positions)
{
    Scene_Information scene_information (description, std::move(simulation_space), fluid_starting_positions);
    if (scene_information.is_valid() == true) {
        this->available_scenes.push_back(scene_information);
    }
    else {
        std::cout << "ERROR: Scene with description '" << description << "' is invalid." << std::endl;
    }
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
    this->calculate_initial_particle_positions();
    return true;
}

void Scene_Handler::calculate_initial_particle_positions ()
{
    // Free the memory if it was used before.
    if (this->particles != NULL) {
        delete[] this->particles;
        this->particles = NULL;
    }
    // Allocate new memory.
    this->particles = new Particle[this->number_of_particles];
    // In the scene settings we have set a number of particles. Since a scene can have
    // multiple volumes that have to be filled with particles, divide the number of particles evenly.
    if (this->available_scenes[this->current_scene_id].fluid_starting_positions.size() == 1) {
        // We dont need to divide.
        this->available_scenes[this->current_scene_id].fluid_starting_positions[0].fill_with_particles(
            this->number_of_particles, this->particles
        );
    }
    else {
        // We need to divide. Divide evenly by taking the volume of each cuboid into account.
        float total_volume = 0.0f;
        for (int i = 0; i < this->available_scenes[this->current_scene_id].fluid_starting_positions.size(); i++) {
            total_volume += this->available_scenes[this->current_scene_id].fluid_starting_positions.at(i).get_volume();
        }
        unsigned int offset = 0;
        for (int i = 0; i < this->available_scenes[this->current_scene_id].fluid_starting_positions.size(); i++) {
            unsigned int partial_number_of_particles;
            if (i < (this->available_scenes[this->current_scene_id].fluid_starting_positions.size() - 1)) {
                partial_number_of_particles = (unsigned int)(this->number_of_particles * (
                    this->available_scenes[this->current_scene_id].fluid_starting_positions.at(i).get_volume() / total_volume
                ));
            }
            else {
                partial_number_of_particles = this->number_of_particles - offset;
            }
            this->available_scenes[this->current_scene_id].fluid_starting_positions[0].fill_with_particles(
                this->number_of_particles, this->particles
            );
            offset += partial_number_of_particles;
        }
    }
}

Cuboid* Scene_Handler::get_pointer_to_simulation_space ()
{
    return &this->available_scenes[this->current_scene_id].simulation_space;
}

std::vector<Cuboid>* Scene_Handler::get_pointer_to_fluid_starting_positions ()
{
    return &this->available_scenes[this->current_scene_id].fluid_starting_positions;
}

void Scene_Handler::print_information () 
{
    std::cout << std::endl << "Available scenes:" << std::endl;
    for (int i = 0; i < this->available_scenes.size(); i++) {
        std::cout << "(" << i + 1 << ") ";
        this->available_scenes[i].print_information();
    }
    std::cout << std::endl;
}