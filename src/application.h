#include "application_handler/application_handler.h"


// This enum type is used for the different input contexts.
enum Input_Behavior {
    INPUT_BEHAVIOR_IDLE,
    INPUT_BEHAVIOR_SIMULATION
};

// We need one global object of the application handler.
extern Application_Handler application_handler;

// Our main application.
// Within this application a while-loop is running until the application
// is terminated.
void rtgp_application();


// Some useful functions for the key-reactions.
void reload_scene ();
void switch_scene (int scene_id);
void pause_resume_simulation ();
void exit_application ();