#pragma once

#include <string>

// An input consists of a reaction that gets executed when the specified
// key is pressed. The description is used to describe the reaction of the 
// behaviour for the user.
class Input {
    private:
        int key;
        void (*reaction) ();
        std::string description;

        // The function glfwGetKeyName does not work for every key
        // and also returns the names in lowercase. So its simpler 
        // to implement it by ourselves instead of making glfwGetKeyName
        // work for our usecase.
        static std::string get_key_name (int key);

    public:
        Input (int key, void (*reaction)(), std::string description);

        // Checks if the given key is the key of this input.
        // If so, it executes the reaction function.
        bool check_and_execute (int pressed_key);

        // Prints the input behaviour to the console.
        void print_information ();
};