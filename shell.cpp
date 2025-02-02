#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <filesystem>
#include <windows.h>

namespace fs = std::filesystem;

class CommandShell
{
private:
    bool running;
    std::string currentDirectory;
    std::vector<std::string> history;

    std::vector<std::string> parseCommand(const std::string &command)
    {
        std::vector<std::string> args;
        std::stringstream ss(command);
        std::string arg;

        while (ss >> arg)
        {
            args.push_back(arg);
        }
        return args;
    }

    bool handleBuiltIn(const std::vector<std::string> &args)
    {
        if (args.empty())
            return true;

        if (args[0] == "help")
        {
            std::cout << "Commands:\n help\n history\n exit\n cd [dir]\n pwd/cd.\n cls\n";
            return true;
        }
        else if (args[0] == "history")
        {
            for (const auto &cmd : history)
                std::cout << cmd << std::endl;
            return true;
        }
        else if (args[0] == "exit")
        {
            running = false;
            return true;
        }
        else if (args[0] == "cd")
        {
            if (args.size() < 2)
            {
                char *userProfile = nullptr;
                size_t len = 0;
                _dupenv_s(&userProfile, &len, "USERPROFILE");
                if (userProfile)
                {
                    size_t conv;
                    wchar_t path[MAX_PATH];
                    mbstowcs_s(&conv, path, MAX_PATH, userProfile, _TRUNCATE);
                    SetCurrentDirectoryW(path);
                    free(userProfile);
                }
            }
            else
            {
                wchar_t path[MAX_PATH];
                size_t conv;
                mbstowcs_s(&conv, path, MAX_PATH, args[1].c_str(), _TRUNCATE);
                if (!SetCurrentDirectoryW(path))
                {
                    std::cerr << "cd: No such directory: " << args[1] << std::endl;
                }
            }
            currentDirectory = fs::current_path().string();
            return true;
        }
        else if (args[0] == "pwd" || args[0] == "cd.")
        {
            std::cout << currentDirectory << std::endl;
            return true;
        }
        else if (args[0] == "cls")
        {
            system("cls");
            return true;
        }

        return false;
    }

    void executeCmd(const std::vector<std::string> &args)
    {
        std::string cmdStr;
        for (const auto &arg : args)
        {
            cmdStr += arg + " ";
        }

        STARTUPINFOW si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        std::string fullCmd = "/c " + cmdStr;

        std::wstring wcmd(fullCmd.begin(), fullCmd.end());
        wchar_t* wcmdPtr = const_cast<wchar_t*>(wcmd.c_str());

        if (CreateProcessW(
                L"C:\\Windows\\System32\\cmd.exe",
                wcmdPtr,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &si,
                &pi
                ))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            std::cerr << "Command failed: " << cmdStr << std::endl;
        }
    }

public:
    CommandShell() : running(true), currentDirectory(fs::current_path().string())
    {
    }

    void run()
    {
        std::string input;

        while (running)
        {
            std::cout << "\033[1;32m" << currentDirectory << "\033[0m> ";
            std::getline(std::cin, input);

            if (std::cin.eof())
            {
                std::cout << "\nLogging out..." << std::endl;
                break;
            }

            if (input.empty())
                continue;

            history.push_back(input);
            auto args = parseCommand(input);
            if (!handleBuiltIn(args))
            {
                executeCmd(args);
            }
        }
    }
};

int main()
{
    try
    {
        CommandShell shell;
        shell.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}