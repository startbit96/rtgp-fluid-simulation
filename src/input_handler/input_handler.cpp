#include <iostream>
#include <algorithm>

#include "input_handler.h"


Input_Handler::Input_Handler ()
{
    this->active = false;
    this->current_context_id = INPUT_HANDLER_CONTEXT_ID_IDLE;
}

bool Input_Handler::register_input_context (int context_id, std::string description)
{
    if (context_id < 0) {
        // Not allowed.
        std::cout << "context_id needs to be greater than or equal to 0." << std::endl;
        return false;
    }
    if (this->input_contexts.contains(context_id) == true) {
        // Context already exists.
        std::cout << "context_id " << context_id << " already exists." << std::endl;
        return false;
    }
    if (this->input_contexts.insert({context_id, Input_Context(description)}).second == false) {
        std::cout << "An error occured registering context_id " << context_id << "." << std::endl;
    }
    return true;
}

bool Input_Handler::change_input_context (int context_id)
{
    this->clear_pressed_keys();
    if (this->input_contexts.contains(context_id) == true) {
        this->current_context_id = context_id;
        this->active = true;
        return true;
    }
    else {
        this->current_context_id = INPUT_HANDLER_CONTEXT_ID_IDLE;
        this->active = false;
        return false;
    }
}

bool Input_Handler::clear_input_context (int context_id)
{
    if (this->input_contexts.contains(context_id) == true) {
        this->input_contexts.erase(context_id);
        if (context_id == this->current_context_id) {
            this->active = false;
            this->current_context_id = INPUT_HANDLER_CONTEXT_ID_IDLE;
        }
        return true;
    }
    else {
        return false;
    }
}

void Input_Handler::clear_all_input_context () 
{
    this->input_contexts.clear();
    this->active = false;
    this->current_context_id = INPUT_HANDLER_CONTEXT_ID_IDLE;
}

bool Input_Handler::add_input_behaviour (int context_id, int key, void (*reaction)(), std::string description)
{
    if (this->input_contexts.contains(context_id) == true) {
        this->input_contexts[context_id].add_input(
            key, reaction, description
        );
        return true;
    }
    else {
        std::cout << "Can not add key " << key << " to context_id " << context_id << "." << std::endl;
        return false;
    }
}

void Input_Handler::clear_pressed_keys () 
{
    this->pressed_keys.clear();
}

void Input_Handler::check_and_execute ()
{
    if ((this->active == true) && (this->current_context_id != INPUT_HANDLER_CONTEXT_ID_IDLE)) {
        this->input_contexts.at(this->current_context_id).check_and_execute(this->pressed_keys);
        this->pressed_keys.clear();
    }
}

void Input_Handler::print_information ()
{
    std::cout << std::endl;
    for (auto& [context_id, input_context]: this->input_contexts) {
        input_context.print_information();
        std::cout << std::endl;
    }
}