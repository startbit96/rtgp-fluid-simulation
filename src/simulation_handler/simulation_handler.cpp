#include "simulation_handler.h"

#include <iostream>
#include <fstream>

#include "../utils/debug.h"


Simulation_Handler::Simulation_Handler()
{
    this->is_running = true;
    this->current_scene_id = -1;
    this->next_scene_id = -1;
}

void Simulation_Handler::register_new_scene (   std::string description,
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

bool Simulation_Handler::delete_scene (int scene_id) 
{
    if (scene_id > this->available_scenes.size() - 1) {
        return false;
    }
    this->available_scenes.erase(this->available_scenes.begin() + scene_id);
    return true;
}

void Simulation_Handler::delete_all_scenes () 
{
    this->available_scenes.clear();
}

bool Simulation_Handler::load_scene ()
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
    this->particle_system.generate_initial_particles(this->available_scenes[current_scene_id].fluid_starting_positions);
    this->particle_system.initialize_spatial_grid(this->available_scenes[current_scene_id].simulation_space);
    return true;
}

void Simulation_Handler::print_scene_information () 
{
    std::cout << std::endl << "Available scenes:" << std::endl;
    for (int i = 0; i < this->available_scenes.size(); i++) {
        std::cout << "(" << i + 1 << ") ";
        this->available_scenes[i].print_information();
    }
    std::cout << std::endl;
}

void Simulation_Handler::toggle_pause_resume_simulation ()
{
    this->is_running = !this->is_running;
}

void Simulation_Handler::simulate ()
{
    if (this->is_running == true) {
        this->particle_system.simulate();
    }
}

Cuboid* Simulation_Handler::get_pointer_to_simulation_space ()
{
    return &this->available_scenes[this->current_scene_id].simulation_space;
}

std::vector<Cuboid>* Simulation_Handler::get_pointer_to_fluid_starting_positions ()
{
    return &this->available_scenes[this->current_scene_id].fluid_starting_positions;
}

Particle_System* Simulation_Handler::get_pointer_to_particle_system ()
{
    return &this->particle_system;
}

glm::vec3 Simulation_Handler::get_current_point_of_interest ()
{
    return this->available_scenes[this->current_scene_id].simulation_space.get_point_of_interest();
}