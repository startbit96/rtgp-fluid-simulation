#pragma once

#include <string>
#include <vector>

// The scene informations are not hard-coded and then compiled
// but they are provided by an json-file that will be read in when 
// the application starts.
// https://github.com/nlohmann/json
#include <nlohmann/json.hpp>
using json = nlohmann::json;


class Scene_Information {
    private:

    public:
        std::string description;
        std::string filepath_simulation_space;
        std::vector<std::string> filepaths_fluids;
        std::vector<std::string> filepaths_obstacles;

        Scene_Information ();
        Scene_Information ( std::string description, 
                            std::string filepath_simulation_space,
                            std::vector<std::string> filepaths_fluids,
                            std::vector<std::string> filepaths_obstacles);

        void print_information ();
};


// In order to parse the json-file into an Scene_Information object,
// we need to define the following function.
// For more information see the GitHub-Repository:
// https://github.com/nlohmann/json
void from_json (const json& json_file, Scene_Information& scene_information);