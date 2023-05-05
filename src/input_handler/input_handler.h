#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "input_context.h"

#define INPUT_HANDLER_CONTEXT_ID_IDLE       -1

class Input_Handler {
    private:
        
    public:
        bool active;
        int current_context_id;
        std::unordered_map<int, Input_Context> input_contexts;
        std::vector<int> pressed_keys;

        Input_Handler();

        bool register_input_context (int context_id, std::string description);
        bool change_input_context (int context_id);
        bool clear_input_context (int context_id);
        void clear_all_input_context ();
        bool add_input_behaviour (int context_id, int key, void (*reaction)(), std::string description);
        void clear_pressed_keys ();
        void check_and_execute ();
        void print_information ();
};