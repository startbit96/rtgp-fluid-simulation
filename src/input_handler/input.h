#pragma once

#include <string>
#include <vector>

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

        // For the visualization handler to be able to print the key bindings in a table we will need to 
        // create a key - reaction string pair. Using this function the input adds himself to this list.
        void add_key_binding_to_list (std::vector<std::string> &key_list, std::vector<std::string> &reaction_description_list);
};