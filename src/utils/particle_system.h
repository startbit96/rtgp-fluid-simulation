#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <mutex>

#include "particle.h"
#include "cuboid.h"


// The number of initial particles depends on the fluids cuboids
// and the distance between the initial particles. This distance can
// be modified during runtime. Define here the min and max distance and
// also the factor. Make sure that the values for init, min and max are 
// within the power series of the inc factor.
// Also note that if the factor is 2 that does not mean the number of 
// particles doubles but since we are in 3D it is 2^3 = 8 times the 
// number of particles.
#define PARTICLE_INITIAL_DISTANCE_INIT          0.064f * sqrt(2)
#define PARTICLE_INITIAL_DISTANCE_MIN           0.008f
#define PARTICLE_INITIAL_DISTANCE_MAX           0.256f
#define PARTICLE_INITIAL_DISTANCE_INC_FACTOR    sqrt(2)
// SPH related defines. Define here the initial values for the simulation.
// Define here also the min, max values and the step size for the imgui sliders.
// Particles mass in kg.
#define SPH_PARTICLE_MASS                       0.02f
#define SPH_PARTICLE_MASS_MIN                   0.005f
#define SPH_PARTICLE_MASS_MAX                   0.1f
#define SPH_PARTICLE_MASS_STEP                  0.005f
// Rest density of the fluid in kg/m^3.
#define SPH_REST_DENSITY                        0.0f
#define SPH_REST_DENSITY_MIN                    0.0f
#define SPH_REST_DENSITY_MAX                    1100.00f
#define SPH_REST_DENSITY_STEP                   0.1f
// Gas constant in J /(kg * K) on Nm / kg.
#define SPH_GAS_CONSTANT                        0.14f
#define SPH_GAS_CONSTANT_MIN                    0.000001f
#define SPH_GAS_CONSTANT_MAX                    20.0f
#define SPH_GAS_CONSTANT_STEP                   0.005f
// Viscosity in N s / m^2 or Pa * s.
#define SPH_VISCOSITY                           0.5f
#define SPH_VISCOSITY_MIN                       0.00001f
#define SPH_VISCOSITY_MAX                       10.0f
#define SPH_VISCOSITY_STEP                      0.5f
// Gravity mode defines.
#define SPH_GRAVITY_MAGNITUDE                   9.8f
#define GRAVITY_MODE_ROT_SWITCH_TIME            200
// Collision handling.
#define SPH_COLLISION_REFLEXION_DAMPING             0.5f
#define SPH_COLLISION_REFLEXION_DAMPING_MIN         0.0f
#define SPH_COLLISION_REFLEXION_DAMPING_MAX         1.0f
#define SPH_COLLISION_REFLEXION_DAMPING_STEP        0.005f
// Damping constant for the spring damper system.
#define SPH_COLLISION_FORCE_DAMPING                 5.0f
#define SPH_COLLISION_FORCE_DAMPING_MIN             0.1f
#define SPH_COLLISION_FORCE_DAMPING_MAX             100.0f
#define SPH_COLLISION_FORCE_DAMPING_STEP            0.05f
// Spring constant for the spring damper system.
#define SPH_COLLISION_FORCE_SPRING_CONSTANT         1000.0f
#define SPH_COLLISION_FORCE_SPRING_CONSTANT_MIN     1.0f
#define SPH_COLLISION_FORCE_SPRING_CONSTANT_MAX     2000.0f
#define SPH_COLLISION_FORCE_SPRING_CONSTANT_STEP    0.1f
// When should the collision force be applied? (only for the force method)
#define SPH_COLLISION_FORCE_DISTANCE_TOLERANCE      0.1f
#define SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_MIN  0.01f
#define SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_MAX  0.5f
#define SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_STEP 0.001f
// Simulation time defines.
#define SPH_SIMULATION_TIME_STEP                0.05f
// Multithreading defines.
#define SIMULATION_NUMBER_OF_THREADS            8
#define SIMULATION_NUMBER_OF_THREADS_MIN        1
#define SIMULATION_NUMBER_OF_THREADS_MAX        8


// Gravity modes for different gravity vectors.
enum Gravity_Mode
{
    GRAVITY_OFF,
    GRAVITY_NORMAL,
    GRAVITY_ROT_90,
    GRAVITY_WAVE,
    _GRAVITY_MODE_COUNT
};

inline const char* to_string (Gravity_Mode gravity_mode)
{
    switch (gravity_mode) {
        case GRAVITY_OFF:       return "GRAVITY OFF";
        case GRAVITY_NORMAL:    return "GRAVITY NORMAL (-Y)";
        case GRAVITY_ROT_90:    return "GRAVITY SWITCH BETWEEN X and Y";
        case GRAVITY_WAVE:      return "GRAVITY WAVE";
        default:                return "unknown gravity mode";
    }
}

// What calculation mode to use?
enum Computation_Mode
{
    COMPUTATION_MODE_BRUTE_FORCE,
    COMPUTATION_MODE_SPATIAL_GRID,
    _COMPUTATION_MODE_COUNT
};

inline const char* to_string (Computation_Mode computation_mode)
{
    switch (computation_mode) {
        case COMPUTATION_MODE_BRUTE_FORCE:      return "BRUTE FORCE";
        case COMPUTATION_MODE_SPATIAL_GRID:     return "SPATIAL GRID";
        default:                                return "unknown computation mode";
    }
}

// Particle System.
class Particle_System 
{
    private:
        // Render related.
        GLuint vertex_array_object;
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        std::vector<unsigned int> particle_indices;

        // Settings.
        float particle_initial_distance;
        float sph_kernel_radius;
        unsigned int simulation_step;
        Cuboid* simulation_space;

        // Calculates the kernels radius based on the initial distance of the particles.
        void calculate_kernel_radius ();

        // Kernels for the SPH method.
        // Some helper variables so that we do not have to calculate the coefficients
        // and some other variables everytime for each particle.
        // The here saved coefficients only depend on the kernels radius, so everytime
        // the kernel radius changes, the coefficients have to be recalculated.
        float kernel_radius_squared;
        float coefficient_kernel_w_poly6;
        float coefficient_kernel_w_poly6_gradient;
        float coefficient_kernel_w_poly6_laplacian;
        float coefficient_kernel_w_spiky_gradient;
        float coefficient_kernel_w_viscosity_laplacian;
        // Kernel functions.
        float kernel_w_poly6 (glm::vec3 distance_vector);
        glm::vec3 kernel_w_poly6_gradient (glm::vec3 distance_vector);
        float kernel_w_poly6_laplacian (glm::vec3 distance_vector);
        glm::vec3 kernel_w_spiky_gradient (glm::vec3 distance_vector);
        float kernel_w_viscosity_laplacian (glm::vec3 distance_vector);

        // Returns the gravity vector based on the selected gravity mode.
        glm::vec3 get_gravity_vector ();

        // Collision handling.
        void resolve_collision_relfexion_method (Particle& particle);
        glm::vec3 resolve_collision_force_method (Particle particle);

        // Multithreading.
        void parallel_for (void (Particle_System::* function)(unsigned int, unsigned int), int number_of_elements);
        void parallel_for_grid (void (Particle_System::* function)(unsigned int, unsigned int));

        // Brute force implementation (used also for the multithreading variant).
        // Note for the following functions: index_end is included in the for loop.
        void calculate_density_pressure_brute_force (unsigned int index_start, unsigned int index_end);
        void calculate_acceleration_brute_force (unsigned int index_start, unsigned int index_end);
        void calculate_verlet_step_brute_force (unsigned int index_start, unsigned int index_end);
        void simulate_brute_force ();

        // Implementation using a spatial grid.
        glm::vec3 particle_offset;
        int number_of_cells;
        int number_of_cells_x;
        int number_of_cells_y;
        int number_of_cells_z;
        void calculate_number_of_grid_cells ();
        std::vector<std::vector<Particle>> spatial_grid;
        std::vector<std::unique_ptr<std::mutex>> mutex_spatial_grid;
        int discretize_value (float value);
        int get_grid_key (glm::vec3 position);
        std::vector<int> get_neighbor_cells_indices (glm::vec3 position); 
        void generate_spatial_grid (unsigned int index_start, unsigned int index_end);
        void calculate_density_pressure_spatial_grid (unsigned int index_start, unsigned int index_end);
        void calculate_acceleration_spatial_grid (unsigned int index_start, unsigned int index_end);
        void calculate_verlet_step_spatial_grid (unsigned int index_start, unsigned int index_end);
        void update_particle_vector ();
        void simulate_spatial_grid ();

    public:
        std::vector<Particle> particles;
        unsigned int number_of_particles;
        // The visualization handler will show the number of particles at the title
        // of the window. In order to make it more readable we save in this string
        // the number of particles with thousand separators, so that the visualization
        // handler can directly use this. The variable is only updated when the number of 
        // particles change.
        std::string number_of_particles_as_string;

        Particle_System ();

        // This function creates the initial particles based on the given spaces that have
        // to be filled an the particle_initial_distance.
        // It also generates the OpenGL needed buffers like the vertex array object.
        void generate_initial_particles (std::vector<Cuboid>& cuboids);
        void set_simulation_space (Cuboid* simulation_space);

        // Note that these functions do not call the generate_initial_particles function, this
        // has to be done by the application. It simply increases / decreases the distance between
        // initial particles so that with the next call of generate_initial_particles it will take effect.
        // The bool value that the function gives back indicates if an increasement / decreasement is
        // possible and if the call to generate_initial_particles makes sense or not.
        bool increase_number_of_particles ();
        bool decrease_number_of_particles ();

        // Simulation fluid settings. The values are public in order to allow imgui to change them.
        float sph_particle_mass;
        float sph_rest_density;
        float sph_gas_constant;
        float sph_viscosity;
        void reset_fluid_attributes ();

        // Simulation collision settings.
        float collision_reflexion_damping;
        float collision_force_damping;
        float collision_force_spring_constant;
        float collision_force_distance_tolerance;
        void reset_collision_attributes ();

        // Change the gravity mode. 
        Gravity_Mode gravity_mode;
        void next_gravity_mode ();
        void change_gravity_mode (Gravity_Mode gravity_mode);

        // Change the computation mode (either brute force or with spatial hash grid).
        Computation_Mode computation_mode;
        void next_computation_mode ();
        void change_computation_mode (Computation_Mode computation_mode);

        // Multithreading.
        int number_of_threads;

        // Simulate the next step. What computation mode is internally used is determined by the 
        // setted computation mode.
        void simulate ();

        // Draws the particles. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer).
        void free_gpu_resources ();
};