#ifndef LinuxShell_H
#define LinuxShell_H

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <unistd.h>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef MAX_BUF
#define MAX_BUF 200
#endif

template<typename T>
class ArgList : public std::list<T>
{
public:
    bool doInBackground = false;
};

class LinuxShell
{
    std::vector<std::string> m_path;
    std::vector<pid_t> m_backgroundProcesses;

    void BackgroundCheck()
    {
        int index = 0;
        std::vector<int> indicesToRemove;
        for(pid_t pid : m_backgroundProcesses)
        {
            int status;
            waitpid(pid, &status, WNOHANG);
            if(WIFEXITED(status))
            {
                indicesToRemove.push_back(index);
            }
            ++index;
        }
        for(int i : indicesToRemove)
        {
            m_backgroundProcesses[index] = m_backgroundProcesses.back();
            m_backgroundProcesses.pop_back();
        }
    }

    std::string VectorToString(std::vector<std::string> vec)
    {
        std::stringstream ss("");
        //ss << "/";
        for(std::string s : vec)
        {
            ss << s << "/" ;
        }
        return ss.str();
    }

    std::vector<std::string> PathStringToVector(std::string pathString)
    {
        std::vector<std::string> pathVec;
        std::stringstream sstream(pathString);
        std::string token;
        while(std::getline(sstream, token, '/'))
        {
            pathVec.push_back(token);
        } 
        return pathVec;
    }

    void MergeVectors(std::vector<std::string> &v1, std::vector<std::string> &v2)
    {
        for(std::string s : v2)
        {
            v1.push_back(s);
        }
    }

    ArgList<std::string> ParseArgs(std::string input);

public:

    LinuxShell(); 

    ~LinuxShell()
    {
        for(pid_t pid : m_backgroundProcesses)
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    int Run();

    std::string Prompt();
};

#endif