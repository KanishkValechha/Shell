#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <stack>
#include <map>
#include <windows.h>
namespace fs = std::filesystem;

class Shell
{
    bool active;
    std::string workDir;
    std::vector<std::string> cmdHistory;
    std::stack<std::string> dirStack;
    std::map<std::string, std::string> aliases;

    std::vector<std::string> tokenize(const std::string &input)
    {
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = false;

        for (size_t i = 0; i < input.length(); i++)
        {
            if (input[i] == '"')
            {
                inQuotes = !inQuotes;
                continue;
            }
            if (input[i] == ' ' && !inQuotes)
            {
                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else
            {
                token += input[i];
            }
        }
        if (!token.empty())
            tokens.push_back(token);
        return tokens;
    }

    bool handleInternal(const std::vector<std::string> &args)
    {
        if (args.empty())
            return true;

        std::string cmd = args[0];
        if (aliases.count(cmd))
        {
            cmd = aliases[cmd];
        }

        if (cmd == "help")
        {
            std::cout << "Available commands:\n"
                      << "help     - Show this help\n"
                      << "history  - Show command history\n"
                      << "exit     - Exit shell\n"
                      << "cd [dir] - Change directory\n"
                      << "pwd      - Print working directory\n"
                      << "ls       - List directory contents\n"
                      << "mkdir    - Create directory\n"
                      << "rm       - Remove file or directory\n"
                      << "cat      - Display file contents\n"
                      << "touch    - Create empty file\n"
                      << "pushd    - Push directory to stack\n"
                      << "popd     - Pop directory from stack\n"
                      << "alias    - Create command alias\n"
                      << "cls      - Clear screen\n";
            return true;
        }

        if (cmd == "history")
        {
            for (const auto &cmd : cmdHistory)
                std::cout << cmd << '\n';
            return true;
        }

        if (cmd == "exit")
        {
            active = false;
            return true;
        }

        if (cmd == "cd")
        {
            if (args.size() < 2)
            {
                char *home = nullptr;
                size_t len;
                _dupenv_s(&home, &len, "USERPROFILE");
                if (home)
                {
                    wchar_t wpath[MAX_PATH];
                    size_t converted;
                    mbstowcs_s(&converted, wpath, MAX_PATH, home, _TRUNCATE);
                    SetCurrentDirectoryW(wpath);
                    free(home);
                }
            }
            else
            {
                wchar_t wpath[MAX_PATH];
                size_t converted;
                mbstowcs_s(&converted, wpath, MAX_PATH, args[1].c_str(), _TRUNCATE);
                if (!SetCurrentDirectoryW(wpath))
                    std::cerr << "Directory not found: " << args[1] << '\n';
            }
            workDir = fs::current_path().string();
            return true;
        }

        if (cmd == "pwd" || cmd == "cd.")
        {
            std::cout << workDir << '\n';
            return true;
        }

        if (cmd == "cls")
        {
            system("cls");
            return true;
        }

        if (cmd == "ls")
        {
            std::string path = args.size() > 1 ? args[1] : ".";
            try
            {
                for (const auto &entry : fs::directory_iterator(path))
                {
                    std::cout << entry.path().filename().string();
                    if (entry.is_directory())
                        std::cout << "/";
                    std::cout << "\n";
                }
            }
            catch (const fs::filesystem_error &e)
            {
                std::cerr << "Error: " << e.what() << '\n';
            }
            return true;
        }

        if (cmd == "mkdir" && args.size() > 1)
        {
            try
            {
                fs::create_directories(args[1]);
            }
            catch (const fs::filesystem_error &e)
            {
                std::cerr << "Error creating directory: " << e.what() << '\n';
            }
            return true;
        }

        if (cmd == "rm" && args.size() > 1)
        {
            try
            {
                fs::remove_all(args[1]);
            }
            catch (const fs::filesystem_error &e)
            {
                std::cerr << "Error removing: " << e.what() << '\n';
            }
            return true;
        }

        if (cmd == "cat" && args.size() > 1)
        {
            std::ifstream file(args[1]);
            if (file)
            {
                std::cout << file.rdbuf();
            }
            else
            {
                std::cerr << "Error: Cannot open file " << args[1] << '\n';
            }
            return true;
        }

        if (cmd == "touch" && args.size() > 1)
        {
            std::ofstream file(args[1], std::ios::app);
            return true;
        }

        if (cmd == "pushd" && args.size() > 1)
        {
            dirStack.push(workDir);
            std::string newDir = args[1];
            if (SetCurrentDirectoryA(newDir.c_str()))
            {
                workDir = fs::current_path().string();
            }
            else
            {
                dirStack.pop();
                std::cerr << "Failed to change directory\n";
            }
            return true;
        }

        if (cmd == "popd")
        {
            if (!dirStack.empty())
            {
                if (SetCurrentDirectoryA(dirStack.top().c_str()))
                {
                    workDir = dirStack.top();
                    dirStack.pop();
                }
                else
                {
                    std::cerr << "Failed to change directory\n";
                }
            }
            else
            {
                std::cerr << "Directory stack is empty\n";
            }
            return true;
        }

        if (cmd == "alias" && args.size() > 2)
        {
            aliases[args[1]] = args[2];
            return true;
        }

        return false;
    }

    void execute(const std::vector<std::string> &args)
    {
        std::string cmd;
        for (const auto &arg : args)
            cmd += arg + " ";

        STARTUPINFOW si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        std::wstring wcmd = L"/c " + std::wstring(cmd.begin(), cmd.end());

        if (CreateProcessW(L"C:\\Windows\\System32\\cmd.exe",
                           &wcmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            std::cerr << "Failed to execute: " << cmd << '\n';
        }
    }

public:
    Shell() : active(true), workDir(fs::current_path().string()) {}

    void run()
    {
        std::string input;
        while (active)
        {
            std::cout << "\033[1;32m" << workDir << "\033[0m> ";
            std::getline(std::cin, input);

            if (std::cin.eof())
            {
                std::cout << "\nLogging out...\n";
                break;
            }

            if (input.empty())
                continue;

            cmdHistory.push_back(input);
            auto args = tokenize(input);
            if (!handleInternal(args))
                execute(args);
        }
    }
};

int main()
{
    try
    {
        Shell().run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}