/**
 * main.cpp

 * Entry point to the program.
 * Initiates a shell object and executes it
 */

#include <iostream>
#include "osh.cpp"

int main(int argc, char* argv[])
{
    int status = status_success;

    if(argc > 1)
    {
        if(0 == strncmp(argv[1], "-v", 3))
        {
            logger.set_logLevel(verbose);
        }
    }

    logger.log(info, "main()::creating Shell \n");

    osh *shell = new osh();

    status = shell->run();

    logger.log(info, "main()::Exit from shell\n");
}
