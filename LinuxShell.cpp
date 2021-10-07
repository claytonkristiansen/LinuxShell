
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
    for(int i = 0; i < input.size(); ++i)
    {
        if(input[i] == '&')
        {
            argList.doInBackground = true;
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
        }
        else if(input[i] == '|')
        {
            argList.piping = true;
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            argList.push_back("|");
        }
        else if(input[i] == '>')
        {
            argList.oRedirection = true;
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            int quoteCount = 0;
            ++i;
            while(quoteCount < 2)
            {
                if(input[i] != '"')
                {
                    ss << input [i];
                }
                else
                {
                    ++quoteCount;
                }
                ++i;
            }
            argList.oRedirectionFile = ss.str();
            ss.str("");
        }
        else if(input[i] == '<')
        {
            argList.iRedirection = true;
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            int quoteCount = 0;
            ++i;
            while(quoteCount < 2)
            {
                if(input[i] != '"')
                {
                    ss << input [i];
                }
                else
                {
                    ++quoteCount;
                }
                ++i;
            }
            argList.iRedirectionFile = ss.str();
            ss.str("");
        }
        else if(inQuote)
        {
            if(input[i] == '"' || input[i] == '\'')
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            else
            {
                ss << input[i];
            }
        }
        else if(input[i] == '"' || input[i] == '\'')
        {
            inQuote = true;
        }
        else if(input[i] == ' ')
        {
            if(ss.str() == "&")
            {
                argList.doInBackground = true;
            }
            else if(ss.str() == "|")
            {
                argList.piping = true;
                argList.push_back(ss.str());
            }
            else
            {
                argList.push_back(ss.str());   
            }
            ss.str("");
        }
        else
        {
            ss << input[i];
        }
    }
    if(ss.str().length() > 0)
    {
        if(ss.str() == "&")
        {
            argList.doInBackground = true;
        }
        else if(ss.str() == "|")
        {
            argList.piping = true;
            argList.push_back(ss.str());
        }
        else
        {
            argList.push_back(ss.str());   
        }
    }
    return argList;
} 

std::vector<char**> LinuxShell::SplitOnPipe(ArgList<std::string> argList)
{
    std::vector<char**> splitList;
    ArgList<std::string> list;
    for(std::string s : argList)
    {
        if(s == "|")
        {
            splitList.push_back(ListToCharArr(list));
            list = ArgList<std::string>();
        }
        else
        {
            list.push_back(s);
        }
    }
    if(list.size() > 0)
    {
        splitList.push_back(ListToCharArr(list));
    }
    return splitList;
}

int LinuxShell::Run()
{
    bool done = false;
    
    while(!done)
    {
        std::string input = Prompt();
        

        ArgList<std::string> argList = ParseArgs(input);
        char** charList = ListToCharArr(argList);
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
        else if(!argList.piping)        //If there is no piping
        {
            pid_t id = fork();
            if(!id)                         //Are the child, and must execute the command
            {
                execvp(charList[0], charList);
            }
            else                            //Are the parent, and must take care of the child now
            {
                if(argList.doInBackground)
                {
                    m_backgroundProcesses.push_back(id);
                }
                else
                {
                    //wait(NULL);
                    int status;
                    if(waitpid(id, &status, 0))
                    {
                        //std::cout << "Killed " << id << "\n";
                    }
                }
            }
        }
        else
        {
            int savedIn = dup(0);
            int savedOut = dup(1);
            std::vector<char**> splitList = SplitOnPipe(argList);
            for(int i = 0; i < splitList.size(); ++i)
            {
                int fds[2];
                pipe(fds);
                pid_t pid = fork();
                if(!pid)
                {
                    char** command = splitList[i];
                    //std::cout << "running " << command[0] << "\n";
                    if(i < splitList.size() - 1)
                    {
                        dup2(fds[1], 1);
                    }
                    execvp(command[0], command);
                }
                else
                {
                    waitpid(pid, NULL, 0);
                    if(i != 0)
                    {          
                        
                    }
                    dup2(fds[0], 0);
                    close(fds[1]);
                }
            }

            dup2(savedIn, 0);       //Restoring stdin
            dup2(savedOut, 1);      //Restoring stdout

            // if(!fork())
            // {
            //     std::vector<char**> splitList = SplitOnPipe(argList);
            //     pid_t id = 0;
            //     int i = 0;
            //     int fds[2];
            //     for(; id == 0 && i < splitList.size() - 1; ++i)
            //     {
            //         std::cout << "splitting\n";
            //         pipe(fds);
            //         id = fork();
            //     }
            //     if(id) {--i;}
            //     wait(NULL);
            //     if(i)
            //     {
            //         std::cout << "redirect stdout\n";
            //         dup2(fds[1], 1);        //redirect stdout
            //     }
            //     if(id)
            //     {
            //         std::cout << "redirect stdin\n";
            //         dup2(fds[0], 0);        //redirect stdin
            //         close(fds[1]);
            //     }
            //     char** command = splitList[splitList.size() - i - 1];
            //     std::cout << "running " << command[0] << "\n";
            //     execvp(command[0], command);
            // }
            // else
            // {
            //     wait(NULL);
            // }

        }
        BackgroundCheck();
    }
    return 0;
}