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

public:

    LinuxShell(); 

    int Run();

    std::string Prompt()
    {
        std::cout << "";
    }
};


#endif