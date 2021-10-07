
#include "LinuxShell.h"

std::string PrintList(std::list<std::string> l)
{
    std::stringstream ss;
    for(std::string s : l)
    {
        ss << s << "\n";
    }
    return ss.str();
}

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

ArgList<std::string> LinuxShell::ParseArgs(std::string input)
{
    ArgList<std::string> argList;
    std::stringstream ss("");
    bool inQuote = false;
    for(char c : input)
    {
        if(inQuote)
        {
            if(c == '"' || c == '\'')
            {
                if(ss.str() == "&")
                {
                    argList.doInBackground = true;
                }
                else
                {
                    argList.push_back(ss.str());   
                }
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
            if(ss.str() == "&")
            {
                argList.doInBackground = true;
            }
            else
            {
                argList.push_back(ss.str());   
            }
            ss.str("");
        }
        else
        {
            ss << c;
        }
    }
    if(ss.str().length() > 0)
    {
        if(ss.str() == "&")
        {
            argList.doInBackground = true;
        }
        else
        {
            argList.push_back(ss.str());   
        }
    }
    return argList;
} 

int LinuxShell::Run()
{
    bool done = false;
    
    while(!done)
    {
        std::string input = Prompt();
        

        ArgList<std::string> argList = ParseArgs(input);

        //BIG IF TIME
        if(argList.front() == "cd")
        {
            argList.pop_front();
            if(argList.front() == "..")
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
                }
                else
                {
                    MergeVectors(m_path, pathVec);
                    chdir(argList.front().c_str());
                    argList.pop_front();
                }
            }
        }
        else if(argList.front() == "pwd")
        {
            char path[MAX_BUF];
            getcwd(path, MAX_BUF);
            std::cout << path << "\n";
        }
        else 
        {
            pid_t id = fork();
            if(!id)          //Are the child, and must execute the command
            {
                std::cout << PrintList(argList);
                done = true;
            }
            else                            //Are the parent, and must take care of the child now
            {
                //std::cout << id << "\n";
                if(argList.doInBackground)
                {
                    m_backgroundProcesses.push_back(id);
                }
                else
                {
                    wait(NULL);
                }
            }
        }
        BackgroundCheck();
    }
    return 0;
}