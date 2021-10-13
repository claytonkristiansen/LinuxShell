
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

void LinuxShell::BackgroundCheck()
{
    int index = 0;
    std::vector<int> indicesToRemove;
    for(pid_t pid : m_backgroundProcesses)
    {
        int status;
        if(waitpid(pid, &status, WNOHANG))
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

std::string LinuxShell::VectorToString(std::vector<std::string> vec)
{
    std::stringstream ss("");
    for(std::string s : vec)
    {
        ss << s << "/" ;
    }
    return ss.str();
}

std::vector<std::string> LinuxShell::PathStringToVector(std::string pathString)
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

char* LinuxShell::StringToChar(std::string s)
{
    int size = s.size();
    char* arg = new char[size + 1];
    int i = 0;
    for(; i < s.size(); ++i)
    {
        arg[i] = s[i];
    }
    arg[i] = '\0';
    return arg;
} 

char** LinuxShell::ListToCharArr(ArgList v)
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
    m_allocatedDoubleCharPointers.push_back(args);
    return args;
}

void LinuxShell::MergeVectors(std::vector<std::string> &v1, std::vector<std::string> &v2)
{
    for(std::string s : v2)
    {
        v1.push_back(s);
    }
}

bool LinuxShell::InputRedirection(ArgList &argList)
{
    for(int i = 0; i < argList.size(); ++i)
    {
        if(argList[i] == "<")
        {
            std::string file = argList[i + 1];
            int fd = open(file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            dup2(fd, 0);
            argList.Remove(i);
            argList.Remove(i);
            return true;
        }
    }
    return false;
}

bool LinuxShell::OutputRedirection(ArgList &argList)
{
    for(int i = 0; i < argList.size(); ++i)
    {
        if(argList[i] == ">")
        {
            std::string file = argList[i + 1]; 
            int fd = open(file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            dup2(fd, 1);
            argList.Remove(i);
            argList.Remove(i);
            return true;
        }
    }
    return false;
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

LinuxShell::~LinuxShell()
{
    for(pid_t pid : m_backgroundProcesses)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    for(char** charPointerArr : m_allocatedDoubleCharPointers)  //Frees all the memory not already freed during runtime
    {
        delete[] charPointerArr;
    }
}

std::string LinuxShell::Prompt()
{
    std::string input;
    char* username = getlogin();    //Gets username
    time_t rawtime;
    time(&rawtime);
    std::string str(asctime(localtime(&rawtime)));  //Gets time string
    str.pop_back();
    std::cout << "\n" << username << "[" << str << "]" << VectorToString(m_path) << "$ ";   //Prints actual prompt
    std::getline(std::cin, input);  //Waits for user input

    return input;
}

int LinuxShell::Run()
{
    bool done = false;
    
    while(!done)
    {
        int savedIn = dup(0);           //Save cin
        int savedOut = dup(1);          //Save cout
        std::string input = Prompt();   //Prompt user and get input
        

        ArgList argList = ParseArgs(input);
        char** charList = ListToCharArr(argList);

        //BIG IF TIME
        if(argList[0] == "cd")          //If command is directory manipulation
        {
            if(argList[1] == ".")
            {
                m_lastDir = m_path;
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
        else if(argList[0] == "pwd")            //Special case for pwd
        {
            char path[MAX_BUF];
            getcwd(path, MAX_BUF);
            std::cout << path << "\n";
        }
        else if(argList.piping)                 //If there is piping
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
        else                                //Default case
        {
            pid_t id = fork();
            if(!id)                                 //Are the child, and must execute the command
            {
                InputRedirection(argList);
                OutputRedirection(argList);
                char** charList = ListToCharArr(argList);
                execvp(charList[0], charList);
            }
            else                                    //Are the parent, and must take care of the child now
            {
                if(argList.doInBackground)
                {
                    m_backgroundProcesses.push_back(id);
                }
                else
                {
                    int status;
                    waitpid(id, &status, 0);
                }
            }
        }
        dup2(savedIn, 0);       //Restoring stdin
        dup2(savedOut, 1);      //Restoring stdout
        BackgroundCheck();
        for(char** charPointerArr : m_allocatedDoubleCharPointers)  //Frees all the char** allocated during this passthrough
        {
            delete[] charPointerArr;
        }
        m_allocatedDoubleCharPointers.clear();
    }
    return 0;
}