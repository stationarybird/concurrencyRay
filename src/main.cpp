#define GL_SILENCE_DEPRECATION
#include "application.hpp"
#include <iostream>

int main()
{
    Application app;
    if (!app.initialize())
    {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    app.run();
    return 0;
}