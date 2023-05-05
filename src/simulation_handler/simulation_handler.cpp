#include "simulation_handler.h"

Simulation_Handler::Simulation_Handler ()
{
    this->is_running = true;
    this->computation_mode = COMPUTATION_MODE_CPU;
    // Load the functions used for calculating the simulation step.
    this->load_calculation_functions();
}

void Simulation_Handler::load_calculation_functions ()
{

}

void Simulation_Handler::calculate_next_simulation_step () 
{
    
}

void Simulation_Handler::change_computation_mode (Computation_Mode computation_mode)
{
    this->computation_mode = computation_mode;
    this->load_calculation_functions();
}

void Simulation_Handler::toggle_computation_mode ()
{
    if (this->computation_mode == COMPUTATION_MODE_CPU) {
        this->change_computation_mode(COMPUTATION_MODE_GPU);
    }
    else {
        this->change_computation_mode(COMPUTATION_MODE_CPU);
    }
}

void Simulation_Handler::pause_simulation ()
{
    this->is_running = false;
}

void Simulation_Handler::resume_simulation ()
{
    this->is_running = true;
}

void Simulation_Handler::toggle_pause_resume_simulation ()
{
    if (this->is_running == true) {
        this->is_running = false;
    }
    else {
        this->is_running = true;
    }
}