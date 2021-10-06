#ifndef LinuxShell_H
#define LinuxShell_H

#include <string>
#include <iostream>
#include <vector>
#include <list>
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
    

    std::list<std::string> ParseArgs(std::string input)
    {
        std::list<std::string> argList;
        std::stringstream ss("");
        bool inQuote = false;
        for(char c : input)
        {
            if(inQuote)
            {
                if(c == '"' || c == '\'')
                {
                    argList.push_back(ss.str());
                    ss.str("");
                }
                else
                {
                    ss << c;
                }
            }
            else if(c == '"' || c == '\'')
            {
                inQuote = true;
            }
            else if(c == ' ')
            {
                argList.push_back(ss.str());
                ss.str("");
            }
            else
            {
                ss << c;
            }
        }
        if(ss.str().length() > 0)
        {
            argList.push_back(ss.str());
        }
        return argList;
    } 

public:

    LinuxShell(); 

    int Run();

    std::string Prompt();
};


#endif