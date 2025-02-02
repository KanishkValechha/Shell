#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <windows.h>
namespace fs = std::filesystem;

class Shell {
    bool active;
    std::string workDir;
    std::vector<std::string> cmdHistory;

    std::vector<std::string> tokenize(const std::string &input) {
        std::vector<std::string> tokens;
        std::stringstream ss(input);
        std::string token;
        while (ss >> token) tokens.push_back(token);
        return tokens;
    }

    bool handleInternal(const std::vector<std::string> &args) {
        if (args.empty()) return true;
        
        if (args[0] == "help") {
            std::cout << "Available commands:\n"
                      << "help     - Show this help\n"
                      << "history  - Show command history\n"
                      << "exit     - Exit shell\n"
                      << "cd [dir] - Change directory\n"
                      << "pwd      - Print working directory\n"
                      << "cls      - Clear screen\n";
            return true;
        }
        
        if (args[0] == "history") {
            for (const auto &cmd : cmdHistory) std::cout << cmd << '\n';
            return true;
        }
        
        if (args[0] == "exit") {
            active = false;
            return true;
        }
        
        if (args[0] == "cd") {
            if (args.size() < 2) {
                char *home = nullptr;
                size_t len;
                _dupenv_s(&home, &len, "USERPROFILE");
                if (home) {
                    wchar_t wpath[MAX_PATH];
                    size_t converted;
                    mbstowcs_s(&converted, wpath, MAX_PATH, home, _TRUNCATE);
                    SetCurrentDirectoryW(wpath);
                    free(home);
                }
            } else {
                wchar_t wpath[MAX_PATH];
                size_t converted;
                mbstowcs_s(&converted, wpath, MAX_PATH, args[1].c_str(), _TRUNCATE);
                if (!SetCurrentDirectoryW(wpath))
                    std::cerr << "Directory not found: " << args[1] << '\n';
            }
            workDir = fs::current_path().string();
            return true;
        }
        
        if (args[0] == "pwd" || args[0] == "cd.") {
            std::cout << workDir << '\n';
            return true;
        }
        
        if (args[0] == "cls") {
            system("cls");
            return true;
        }
        
        return false;
    }

    void execute(const std::vector<std::string> &args) {
        std::string cmd;
        for (const auto &arg : args) cmd += arg + " ";
        
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::wstring wcmd = L"/c " + std::wstring(cmd.begin(), cmd.end());
        
        if (CreateProcessW(L"C:\\Windows\\System32\\cmd.exe",
            &wcmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            std::cerr << "Failed to execute: " << cmd << '\n';
        }
    }

public:
    Shell() : active(true), workDir(fs::current_path().string()) {}
    
    void run() {
        std::string input;
        while (active) {
            std::cout << "\033[1;32m" << workDir << "\033[0m> ";
            std::getline(std::cin, input);
            
            if (std::cin.eof()) {
                std::cout << "\nLogging out...\n";
                break;
            }
            
            if (input.empty()) continue;
            
            cmdHistory.push_back(input);
            auto args = tokenize(input);
            if (!handleInternal(args)) execute(args);
        }
    }
};

int main() {
    try {
        Shell().run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}