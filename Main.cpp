#include "LinuxShell.h"

// bool InputRedirectionM(ArgList &argList)
// {
//     for(int i = 0; i < argList.size(); ++i)
//     {
//         if(argList[i] == "<")
//         {
//             std::string file = argList[i + 1];
//             std::cout << "Redirecting input"; 
//             int fd = open(file.c_str(), O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//             dup2(fd, 0);
//             argList.Remove(i);
//             argList.Remove(i + 1);
//             return true;
//         }
//     }
//     return false;
// }

int main(int argc, char** argv)
{
    // ArgList argList;
    // argList.push_back("grep");
    // argList.push_back("/init");
    // argList.push_back("<");
    // argList.push_back("a");
    // InputRedirectionM(argList);
    // std::string s;
    // std::cin >> s;

    LinuxShell linshell;
    linshell.Run();
    return 0;
}