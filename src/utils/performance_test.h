#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <vector>

// Here we will define the following macro:
// If we want to measure the performance of a given function, we not only execute it,
// but also measure the execution time and save it to a dictionary. This will be used
// to determine the time different functions take as well as to compare e.g., the brute
// force implementation of the SPH algorithm against the spatial grid implementation.

// The dictionary where we will save all the execution times.
extern std::unordered_map<std::string, std::vector<long long>> execution_times;

// The macro / function that measures the execution time and saves it into the dictionary.
#ifdef PERFORMANCE_TEST
#define MEASURE_EXECUTION_TIME(function_to_be_measured)\
    do {\
        auto start = std::chrono::high_resolution_clock::now();\
        function_to_be_measured;\
        auto end = std::chrono::high_resolution_clock::now();\
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();\
        execution_times[#function_to_be_measured].push_back(duration);\
    } while (false)
#else
#define MEASURE_EXECUTION_TIME(function_to_be_measured) function_to_be_measured
#endif

// We will need a function that saves the data in a csv file.
// The application should call this at the end.
inline void save_exection_time_to_csv () 
{
    // Some defines, how we want to setup the csv file.
    const char cell_delimiter = ';';
    const char number_delimiter = ',';

    // Open the file where we want to save the data and check if something went wrong.
    const std::string filename = "./performance_data.csv";
    std::ofstream file (filename);
    if (file.is_open() == false) {
        std::cout << "Failed to open file: '" << filename << "'." << std::endl;
        return;
    }

    // Write the header row.
    file << "function" << cell_delimiter << "execution_times" << std::endl;

    // Write the data rows.
    for (const auto& pair : execution_times) {
        const std::string& function_name = pair.first;
        const std::vector<long long>& times = pair.second;

        // Write the function name.
        file << function_name << cell_delimiter;

        // Write the execution times.
        for (size_t i = 0; i < times.size(); i++) {
            file << times[i];
            if (i != times.size() - 1) {
                file << number_delimiter;
            }
        }

        // Next row.
        file << std::endl;
    }

    // Close the file and inform about the path.
    file.close();
    std::cout << "csv file containing performance measurements saved to: '" << filename << "'" << std::endl;
}