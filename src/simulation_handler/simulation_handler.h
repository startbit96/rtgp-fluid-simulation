#pragma once

enum Computation_Mode {
    COMPUTATION_MODE_CPU,
    COMPUTATION_MODE_GPU
};

class Simulation_Handler {
    private:
        void load_calculation_functions ();

    public:
        bool is_running;
        Computation_Mode computation_mode;

        Simulation_Handler();

        void calculate_next_simulation_step ();

        void change_computation_mode (Computation_Mode computation_mode);
        void toggle_computation_mode ();
        void pause_simulation ();
        void resume_simulation ();
        void toggle_pause_resume_simulation ();
};