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
#include <sys/stat.h>
#include <fcntl.h>


#ifndef MAX_BUF
#define MAX_BUF 200
#endif

class ArgList : public std::vector<std::string>
{
public:
    bool doInBackground = false;
    bool piping = false;
    void Remove(int index)
    {
        for(int i = index; i < this->size() - 1; ++i)
        {
            this->at(i) = this->at(i + 1);
        }
        this->pop_back();
    }
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
            if(waitpid(pid, &status, WNOHANG))
            {
                indicesToRemove.push_back(index);
                //std::cout << "Killed " << pid << "\n";
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

    char* StringToChar(std::string s)
    {
        char* arg = new char[s.size()];
        for(int i = 0; i < s.size(); ++i)
        {
            arg[i] = s[i];
        }
        return arg;
    } 

    char** ListToCharArr(ArgList v)
    {
        char** args = new char*[v.size() + 1];

        int index = 0;
        for(std::string s : v)
        {
            char* arg = StringToChar(s);
            args[index] = arg;
            ++index;
        }
        args[index] = NULL;
        return args;
    }

    void MergeVectors(std::vector<std::string> &v1, std::vector<std::string> &v2)
    {
        for(std::string s : v2)
        {
            v1.push_back(s);
        }
    }

    bool InputRedirection(ArgList &argList)
    {
        for(int i = 0; i < argList.size(); ++i)
        {
            if(argList[i] == "<")
            {
                std::string file = argList[i + 1]; 
                int fd = open(file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                dup2(fd, 0);
                argList.Remove(i);
                argList.Remove(i + 1);
                return true;
            }
        }
        return false;
    }

    bool OutputRedirection(ArgList &argList)
    {
        for(int i = 0; i < argList.size(); ++i)
        {
            if(argList[i] == ">")
            {
                std::string file = argList[i + 1]; 
                int fd = open(file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                dup2(fd, 1);
                argList.Remove(i);
                argList.Remove(i + 1);
                return true;
            }
        }
        return false;
    }

    ArgList ParseArgs(std::string input);

    std::vector<ArgList> SplitOnPipe(ArgList argList);

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