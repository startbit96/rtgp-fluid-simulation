#pragma once

#include <string>

class Input {
    private:

    public:
        int key;
        void (*reaction) ();
        std::string description;

        Input (int key, void (*reaction)(), std::string description);

        bool check_and_execute (int pressed_key);
        void print_information ();

        static std::string get_key_name (int key);
};