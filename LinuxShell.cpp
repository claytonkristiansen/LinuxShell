
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

    std::cout << "\n" << "Shell:" << VectorToString(m_path) << "$ ";
    std::getline(std::cin, input);

    return input;
}

int LinuxShell::Run()
{
    bool done = false;
    while(!done)
    {
        std::string input = Prompt();

        std::list<std::string> argList = ParseArgs(input);

        //BIG IF TIME
        if(argList.front() == "cd")
        {
            argList.pop_front();
            if(argList.front() == ".." && argList.size() == 1)
            {
                m_path.pop_back();
                chdir(VectorToString(m_path).c_str());
            }
            else
            {
                std::vector<std::string> pathVec = PathStringToVector(argList.front());
                if(pathVec.front() == m_path.front())
                {
                    m_path = pathVec;
                    chdir(argList.front().c_str());
                    argList.pop_front();
                    // char path[MAX_BUF];
                    // getcwd(path, MAX_BUF);
                    // std::cout << path << "\n";
                }
                else
                {
                    MergeVectors(m_path, pathVec);
                    chdir(argList.front().c_str());
                    argList.pop_front();
                }
            }
        }
    }
    return 0;
}