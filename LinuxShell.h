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
#include <time.h>


#ifndef MAX_BUF
#define MAX_BUF 200     //Maximum buffer for certain system calls
#endif

//Custom argument list class
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

//Linux shell class
class LinuxShell
{
    std::vector<std::string> m_path;            //Current directory tracker
    std::vector<pid_t> m_backgroundProcesses;   //Active background processes tracker
    std::vector<char**> m_allocatedDoubleCharPointers;  //Contains all allocated char**
    std::vector<std::string> m_homePath;        //Path to /home/
    std::vector<std::string> m_lastDir;         //Path of last visited directory for cd -

    //Attempts to reap all currently running background processes
    void BackgroundCheck();

    //Converts a vector of strings to a single string
    std::string VectorToString(std::vector<std::string> vec);

    //Parses path string into vector of directory names
    std::vector<std::string> PathStringToVector(std::string pathString);

    //Converts a string to a char*
    char* StringToChar(std::string s);

    //Converts an ArgList object to a char**
    char** ListToCharArr(ArgList v);

    //Merges two string vectors
    void MergeVectors(std::vector<std::string> &v1, std::vector<std::string> &v2);

    //Sets up input redirection
    bool InputRedirection(ArgList &argList);

    //Sets up output redirection
    bool OutputRedirection(ArgList &argList);

    //Parses arguments into an ArgList object, additional comments unnecessary if person reading this can code well
    ArgList ParseArgs(std::string input);

    //Splits a string along | characters
    std::vector<ArgList> SplitOnPipe(ArgList argList);

public:
    //LinuxShell constructor
    LinuxShell(); 
    
    //LinusShell destructor
    ~LinuxShell();

    //Prints the prompt to the console and waits for input
    std::string Prompt();

    //Runs main loop
    int Run();
};

#endif