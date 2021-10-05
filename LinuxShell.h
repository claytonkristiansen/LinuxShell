#ifndef LinuxShell_H
#define LinuxShell_H

#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sstream>

#ifndef MAX_BUF
#define MAX_BUF 200
#endif

class LinuxShell
{
    std::vector<std::string> m_path;


    std::string VectorToString(std::vector<std::string> vec)
    {
        std::stringstream ss("");
        for(std::string s : vec)
        {
            ss << "/" << s;
        }
        return ss.str();
    }

    std::vector<std::string> ParseArgs(std::string input)
    {
        std::vector<std::string> vec;
        for(char c : input)
        {
            
        }
    } 

public:

    LinuxShell(); 

    int Run();

    std::string Prompt();
};


#endif