
#include "LinuxShell.h"

LinuxShell::LinuxShell()
{
    char path[MAX_BUF];
    getcwd(path, MAX_BUF);
    std::stringstream sstream(std::string(path));
    std::string token;
    while(std::getline(sstream, token))
    {
        std::cout << "shit";
    } 
}

int LinuxShell::Run()
{
    bool done = false;
    while(!done)
    {
        
    }
}