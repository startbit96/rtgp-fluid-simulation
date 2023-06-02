#include "application.h"

int main(int argc, char* argv[]) 
{
    // Within the application we use relative paths to load objects or 
    // to load the scene configuration.
    // How can we ensure that the working directory is correct and the
    // relative paths are working?
    rtgp_application();
    return 0;
}