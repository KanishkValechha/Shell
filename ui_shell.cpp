#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>
#include <cstdio>

// Global controls
HWND outputWnd, inputWnd, runButton, startShellButton;
const int ID_RUN = 101, ID_INPUT = 102, ID_OUTPUT = 103, ID_START = 104;

// Execute command and capture its output using _wpopen
std::wstring executeCommand(const std::wstring &command)
{
    std::wstring result;
    FILE* pipe = _wpopen(command.c_str(), L"r");
    if (!pipe) return L"Failed to run command.\r\n";
    const int bufSize = 128;
    wchar_t buffer[bufSize];
    while (fgetws(buffer, bufSize, pipe))
    {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
            outputWnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                10, 10, 460, 300, hwnd, (HMENU)ID_OUTPUT, GetModuleHandle(NULL), NULL);
            inputWnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER, // Added WS_BORDER for clarity
                10, 320, 360, 25, hwnd, (HMENU)ID_INPUT, GetModuleHandle(NULL), NULL);
            SetFocus(inputWnd); // new: ensure the input box gets focus
            runButton = CreateWindow(L"BUTTON", L"Run",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                380, 320, 90, 25, hwnd, (HMENU)ID_RUN, GetModuleHandle(NULL), NULL);
            // New: Button to start the shell executable
            startShellButton = CreateWindow(L"BUTTON", L"Start Shell", 
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 
                10, 360, 120, 25, hwnd, (HMENU)ID_START, GetModuleHandle(NULL), NULL);
            break;
        case WM_COMMAND:
            if(LOWORD(wParam) == ID_RUN)
            {
                wchar_t buffer[256];
                GetWindowTextW(inputWnd, buffer, 256); // use GetWindowTextW
                std::wstring command(buffer);
                if (!command.empty())
                {
                    // Debug: Append input command to output control
                    std::wstring debug = L"Input command: " + command + L"\r\n";
                    int len = GetWindowTextLengthW(outputWnd);
                    SendMessageW(outputWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                    SendMessageW(outputWnd, EM_REPLACESEL, 0, (LPARAM)debug.c_str());

                    // Prepend "/c " for cmd.exe execution
                    std::wstring fullCmd = L"cmd.exe /c " + command;
                    std::wstring output = executeCommand(fullCmd);
                    
                    // If no output, insert a debug notice
                    if(output.empty())
                    {
                        output = L"[No output returned]\r\n";
                    }
                    
                    // Append command output to the multi-line edit control
                    len = GetWindowTextLengthW(outputWnd);
                    SendMessageW(outputWnd, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                    SendMessageW(outputWnd, EM_REPLACESEL, 0, (LPARAM)output.c_str());
                    
                    // Clear input text
                    SetWindowTextW(inputWnd, L"");
                }
            }
            else if(LOWORD(wParam) == ID_START)
            {
                // New: Launch shell.exe from the UI
                STARTUPINFOW si = { sizeof(si) };
                PROCESS_INFORMATION pi;
                std::wstring appPath = L"shell.exe"; // adjust path if needed
                if(CreateProcessW(NULL, 
                                  &appPath[0], // mutable pointer to command line
                                  NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                }
                else
                {
                    MessageBoxW(hwnd, L"Failed to launch shell.exe", L"Error", MB_OK | MB_ICONERROR); // changed from MessageBox
                }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t clsName[]  = L"GUIShell";
    WNDCLASS wc = { };
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = clsName;
    RegisterClassW(&wc); // use RegisterClassW
    
    HWND hwnd = CreateWindowExW(
        0,
        clsName,              // wide string class name
        L"GUI Shell",            // wide string window title
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    if (hwnd == NULL)
        return 0;
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// Add this forwarding function:
#ifdef UNICODE
extern "C" int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nCmdShow)
{
    return wWinMain(hInst, hPrev, GetCommandLineW(), nCmdShow);
}
#endif
