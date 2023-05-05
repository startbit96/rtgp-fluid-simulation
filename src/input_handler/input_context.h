#pragma once

#include <string>
#include <vector>

#include "input.h"

class Input_Context {
    private:

    public:
        std::vector<Input> registered_inputs;
        std::string description;

        Input_Context (std::string description = "");

        void add_input (int key, void (*reaction)(), std::string description);
        void clear_inputs ();
        void check_and_execute (std::vector<int> pressed_keys);
        void print_information ();
};