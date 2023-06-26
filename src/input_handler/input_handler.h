#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "input_context.h"

#define INPUT_HANDLER_CONTEXT_ID_IDLE       -1

class Input_Handler {
    private:
        bool active;
        int current_context_id;
        std::unordered_map<int, Input_Context> input_contexts;
        
    public:
        std::vector<int> pressed_keys;

        Input_Handler();

        // An input context is the behaviour of the input handler depending on different
        // scenarios. E.g., a scenario in a game would be "main menu" and "in-game".
        bool register_input_context (int context_id, std::string description);
        bool change_input_context (int context_id);
        bool clear_input_context (int context_id);
        void clear_all_input_context ();
        // Add a key-reaction-description pair to the specified input context.
        bool add_input_behaviour (int context_id, int key, void (*reaction)(), std::string description);
        void clear_pressed_keys ();
        // Passes the vector of the pressed keys down to the active input context.
        void check_and_execute ();
        void print_information ();
        // For the visualization handler to be able to print the key bindings in a table we will need to 
        // create a key - reaction string pair for a given input context. 
        bool create_key_binding_list (int context_id, std::vector<std::string> &key_list, std::vector<std::string> &reaction_description_list);
};