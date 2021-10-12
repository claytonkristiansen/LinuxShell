
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
        if(token == "home")
        {
            m_homePath = m_path;
        }
    }
}

std::string LinuxShell::Prompt()
{
    std::string input;
    char* username = getlogin();
    time_t rawtime;
    time(&rawtime);
    std::string str(asctime(localtime(&rawtime)));
    str.pop_back();
    std::cout << "\n" << username << "[" << str << "]" << VectorToString(m_path) << "$ ";
    std::getline(std::cin, input);

    return input;
}

ArgList LinuxShell::ParseArgs(std::string input)
{
    ArgList argList;
    std::stringstream ss("");
    bool inQuote = false;
    for(int i = 0; i < input.size(); ++i)
    {
        if(inQuote)
        {
            if(input[i] == '"' || input[i] == '\'')
            {
                argList.push_back(ss.str());   
                ss.str("");
                inQuote = false;
            }
            else
            {
                ss << input[i];
            }
        }
        else if(input[i] == '&')
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
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            argList.push_back(">");
        }
        else if(input[i] == '<')
        {
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
            argList.push_back("<");
        }
        else if(input[i] == '"' || input[i] == '\'')
        {
            inQuote = true;
        }
        else if(input[i] == ' ')
        {
            if(ss.str().size() > 0)
            {
                argList.push_back(ss.str());   
                ss.str("");
            }
        }
        else
        {
            ss << input[i];
        }
    }
    if(ss.str().length() > 0)
    {
        argList.push_back(ss.str());   
    }
    return argList;
} 

std::vector<ArgList> LinuxShell::SplitOnPipe(ArgList argList)
{
    std::vector<ArgList> splitList;
    ArgList list;
    for(std::string s : argList)
    {
        if(s == "|")
        {
            splitList.push_back(list);
            list = ArgList();
        }
        else
        {
            list.push_back(s);
        }
    }
    if(list.size() > 0)
    {
        splitList.push_back(list);
    }
    return splitList;
}

int LinuxShell::Run()
{
    bool done = false;
    
    while(!done)
    {
        int savedIn = dup(0);
        int savedOut = dup(1);
        std::string input = Prompt();
        

        ArgList argList = ParseArgs(input);
        char** charList = ListToCharArr(argList);

        // InputRedirection(argList);
        // OutputRedirection(argList);

        //BIG IF TIME
        if(argList[0] == "cd")
        {
            if(argList[1] == ".")
            {
                m_lastDir = m_path;
                //Do nothing
            }
            else if(argList[1] == "..")
            {
                m_lastDir = m_path;
                m_path.pop_back();
                chdir(VectorToString(m_path).c_str());
            }
            else if(argList[1] == "../../")
            {
                m_lastDir = m_path;
                m_path.pop_back();
                m_path.pop_back();
                chdir(VectorToString(m_path).c_str());
            }
            else if(argList[1] == "/home/")
            {
                m_lastDir = m_path;
                m_path = m_homePath;
                chdir(VectorToString(m_path).c_str());
            }
            else if(argList[1] == "-")
            {
                std::vector<std::string> tempDir = m_path;
                m_path = m_lastDir;
                m_lastDir = tempDir;
                chdir(VectorToString(m_path).c_str());
            }
            else
            {
                std::vector<std::string> pathVec = PathStringToVector(argList[1]);
                if(pathVec[0] == m_path[0])
                {
                    m_path = pathVec;
                    chdir(argList[1].c_str());
                }
                else
                {
                    MergeVectors(m_path, pathVec);
                    chdir(argList[1].c_str());
                }
            }
        }
        else if(argList[0] == "pwd")
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
                InputRedirection(argList);
                OutputRedirection(argList);
                char** charList = ListToCharArr(argList);
                //std::cout << "Running " << charList[2] << " " << charList[1] << " " << charList[2] << "\n";
                execvp(charList[0], charList);
                //std::cout << "Ran " << charList[0] << "\n";
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
        else            //with piping
        {
            std::vector<ArgList> splitList = SplitOnPipe(argList);
            for(int i = 0; i < splitList.size(); ++i)
            {
                int fds[2];
                pipe(fds);
                pid_t pid = fork();
                if(!pid)
                {
                    if(i == 0)
                    {
                        InputRedirection(splitList[i]);
                    }
                    if(i < splitList.size() - 1)
                    {
                        dup2(fds[1], 1);
                    }
                    else
                    {
                        OutputRedirection(splitList[i]);
                    }
                    char** command = ListToCharArr(splitList[i]);
                    execvp(command[0], command);
                }
                else
                {
                    if(i == splitList.size() - 1)
                    {
                        waitpid(pid, NULL, 0);
                    }
                    dup2(fds[0], 0);
                    close(fds[1]);
                }
            }
        }
        dup2(savedIn, 0);       //Restoring stdin
        dup2(savedOut, 1);      //Restoring stdout
        BackgroundCheck();
    }
    return 0;
}