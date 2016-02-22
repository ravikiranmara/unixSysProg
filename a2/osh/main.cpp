/**
 * main.cpp

 * Entry point to the program.
 * Initiates a shell object and executes it
 */

#include <iostream>
#include "osh.cpp"

int main(int argc, char* argv[])
{
    osh *shell = new osh();

    return shell->run();
}
