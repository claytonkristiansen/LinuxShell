
#include "LinuxShell.h"

LinuxShell::LinuxShell()
{
    char path[MAX_BUF];
    getcwd(path, MAX_BUF);
    std::string pathString = std::string(path);
    std::stringstream sstream(pathString);
    std::string token;
    while(std::getline(sstream, token, '/'))
    {
        m_path.push_back(token);
    } 
}

std::string LinuxShell::Prompt()
{
    std::string input;

    std::cout << "Shell:" << VectorToString(m_path) << "$ ";
    std::cin >> input;

    return input;
}

int LinuxShell::Run()
{
    bool done = false;
    while(!done)
    {
        std::string input = Prompt();
        std::cout << input << "\n";

        //BIG IF TIME
        if(input == "cd")
        {
            chdir()
        }
    }
    return 0;
}