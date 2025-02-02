#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <string>
#include <cstdio>

HWND outputWnd, inputWnd, runBtn, shellBtn;
const int ID_RUN = 101, ID_INPUT = 102, ID_OUTPUT = 103, ID_START = 104;

std::wstring executeCommand(const std::wstring &cmd) {
    std::wstring result;
    FILE* pipe = _wpopen(cmd.c_str(), L"r");
    if (!pipe) return L"Failed to run command.\r\n";
    
    wchar_t buffer[128];
    while (fgetws(buffer, 128, pipe))
        result += buffer;
    _pclose(pipe);
    return result;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
    case WM_CREATE:
        outputWnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 460, 300, hwnd, (HMENU)ID_OUTPUT, GetModuleHandle(NULL), NULL);
        
        inputWnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 320, 360, 25, hwnd, (HMENU)ID_INPUT, GetModuleHandle(NULL), NULL);
        SetFocus(inputWnd);
        
        runBtn = CreateWindowW(L"BUTTON", L"Run",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            380, 320, 90, 25, hwnd, (HMENU)ID_RUN, GetModuleHandle(NULL), NULL);
        
        shellBtn = CreateWindowW(L"BUTTON", L"Start Shell",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 360, 120, 25, hwnd, (HMENU)ID_START, GetModuleHandle(NULL), NULL);
        break;

    case WM_COMMAND:
        if(LOWORD(wParam) == ID_RUN) {
            wchar_t buffer[256];
            GetWindowTextW(inputWnd, buffer, 256);
            std::wstring command(buffer);
            
            if (!command.empty()) {
                std::wstring debug = L"Command: " + command + L"\r\n";
                int len = GetWindowTextLengthW(outputWnd);
                SendMessageW(outputWnd, EM_SETSEL, len, len);
                SendMessageW(outputWnd, EM_REPLACESEL, 0, (LPARAM)debug.c_str());

                std::wstring output = executeCommand(L"cmd.exe /c " + command);
                if(output.empty()) output = L"[No output]\r\n";
                
                len = GetWindowTextLengthW(outputWnd);
                SendMessageW(outputWnd, EM_SETSEL, len, len);
                SendMessageW(outputWnd, EM_REPLACESEL, 0, (LPARAM)output.c_str());
                SetWindowTextW(inputWnd, L"");
            }
        }
        else if(LOWORD(wParam) == ID_START) {
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            std::wstring shellPath = L"shell.exe";
            
            if(CreateProcessW(NULL, &shellPath[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            } else {
                MessageBoxW(hwnd, L"Failed to launch shell", L"Error", MB_OK | MB_ICONERROR);
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    WNDCLASS wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ShellGUI";
    RegisterClassW(&wc);
    
    HWND hwnd = CreateWindowExW(0, L"ShellGUI", L"GUI Shell",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        NULL, NULL, hInstance, NULL);
    
    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

#ifdef UNICODE
extern "C" int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    return wWinMain(hInst, hPrev, GetCommandLineW(), nShow);
}
#endif
