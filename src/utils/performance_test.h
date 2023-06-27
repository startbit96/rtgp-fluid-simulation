#include <iostream>
#include <chrono>

// Here we will define the following macro:
// If we want to measure the performance of a given function, we not only execute it,
// but also measure the execution time and print it to the terminal. This will be used
// to determine the time different functions take as well as to compare e.g., the brute
// force implementation of the SPH algorithm against the spatial grid implementation.

#ifdef PERFORMANCE_TEST
#define MEASURE_EXECUTION_TIME(function_to_be_measured)\
    do {\
        auto start = std::chrono::high_resolution_clock::now();\
        function_to_be_measured;\
        auto end = std::chrono::high_resolution_clock::now();\
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();\
        std::cout << "Execution time of " << #function_to_be_measured << ": " << duration << " ms" << std::endl;\
    } while (false)
#else
#define MEASURE_EXECUTION_TIME(function_to_be_measured) function_to_be_measured
#endif