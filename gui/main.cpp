#include <windows.h>
#include <string>
#include <vector>

#include "activity_manager.h"

const char g_szClassName[] = "myWindowClass";
std::string contentToDisplay;

// Control IDs
#define ID_EDIT_ACTIVITY    1001
#define ID_BTN_IMPORTANT    1002
#define ID_BTN_NOT_IMPORTANT 1003
#define ID_BTN_URGENT       1004
#define ID_BTN_NOT_URGENT   1005
#define ID_BTN_ADD          1006
#define ID_BTN_SHOW_RESULT  1007
#define ID_TEXT_RESULT      1008

// Global variables
ActivityManager manager;
HWND hEditActivity, hBtnImportant, hBtnNotImportant, hBtnUrgent, hBtnNotUrgent;
HWND hBtnAdd, hBtnShowResult, hTextResult;
bool isImportant = false, isUrgent = false;

// Window Proc
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create input field for activity name
        CreateWindowA("STATIC", "Nama Aktivitas:", WS_VISIBLE | WS_CHILD,
                     20, 20, 120, 20, hwnd, NULL, NULL, NULL);
        
        hEditActivity = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
                                     20, 45, 300, 25, hwnd, (HMENU)ID_EDIT_ACTIVITY, NULL, NULL);

        // Important buttons
        CreateWindowA("STATIC", "Apakah penting?", WS_VISIBLE | WS_CHILD,
                     20, 85, 120, 20, hwnd, NULL, NULL, NULL);
        
        hBtnImportant = CreateWindowA("BUTTON", "Ya, Penting", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     20, 110, 100, 30, hwnd, (HMENU)ID_BTN_IMPORTANT, NULL, NULL);
        
        hBtnNotImportant = CreateWindowA("BUTTON", "Tidak Penting", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                        130, 110, 100, 30, hwnd, (HMENU)ID_BTN_NOT_IMPORTANT, NULL, NULL);

        // Urgent buttons
        CreateWindowA("STATIC", "Apakah mendesak?", WS_VISIBLE | WS_CHILD,
                     20, 155, 120, 20, hwnd, NULL, NULL, NULL);
        
        hBtnUrgent = CreateWindowA("BUTTON", "Ya, Mendesak", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                  20, 180, 100, 30, hwnd, (HMENU)ID_BTN_URGENT, NULL, NULL);
        
        hBtnNotUrgent = CreateWindowA("BUTTON", "Tidak Mendesak", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                     130, 180, 100, 30, hwnd, (HMENU)ID_BTN_NOT_URGENT, NULL, NULL);

        // Add and Show Result buttons
        hBtnAdd = CreateWindowA("BUTTON", "Tambah Aktivitas", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                               20, 230, 120, 30, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
        
        hBtnShowResult = CreateWindowA("BUTTON", "Tampilkan Hasil", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                      150, 230, 120, 30, hwnd, (HMENU)ID_BTN_SHOW_RESULT, NULL, NULL);

        // Result display area
        hTextResult = CreateWindowA("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | 
                                   ES_MULTILINE | ES_READONLY,
                                   20, 280, 520, 200, hwnd, (HMENU)ID_TEXT_RESULT, NULL, NULL);
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_BTN_IMPORTANT:
            isImportant = true;
            SetWindowTextA(hBtnImportant, "[✓] Ya, Penting");
            SetWindowTextA(hBtnNotImportant, "Tidak Penting");
            break;
            
        case ID_BTN_NOT_IMPORTANT:
            isImportant = false;
            SetWindowTextA(hBtnImportant, "Ya, Penting");
            SetWindowTextA(hBtnNotImportant, "[✓] Tidak Penting");
            break;
            
        case ID_BTN_URGENT:
            isUrgent = true;
            SetWindowTextA(hBtnUrgent, "[✓] Ya, Mendesak");
            SetWindowTextA(hBtnNotUrgent, "Tidak Mendesak");
            break;
            
        case ID_BTN_NOT_URGENT:
            isUrgent = false;
            SetWindowTextA(hBtnUrgent, "Ya, Mendesak");
            SetWindowTextA(hBtnNotUrgent, "[✓] Tidak Mendesak");
            break;
            
        case ID_BTN_ADD:
        {
            char activityName[256];
            GetWindowTextA(hEditActivity, activityName, sizeof(activityName));
            
            if (strlen(activityName) > 0) {
                manager.addActivity(std::string(activityName), isImportant, isUrgent);
                SetWindowTextA(hEditActivity, ""); // Clear input
                
                // Reset selections
                isImportant = false;
                isUrgent = false;
                SetWindowTextA(hBtnImportant, "Ya, Penting");
                SetWindowTextA(hBtnNotImportant, "Tidak Penting");
                SetWindowTextA(hBtnUrgent, "Ya, Mendesak");
                SetWindowTextA(hBtnNotUrgent, "Tidak Mendesak");
                
                MessageBoxA(hwnd, "Aktivitas berhasil ditambahkan!", "Info", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(hwnd, "Mohon masukkan nama aktivitas!", "Peringatan", MB_OK | MB_ICONWARNING);
            }
        }
        break;
        
        case ID_BTN_SHOW_RESULT:
        {
            manager.categorizeActivities();
            std::string result = manager.getQuadrantAsString() + "\n" + manager.getRecommendations();
            SetWindowTextA(hTextResult, result.c_str());
        }
        break;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
    break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Main program (WinMain)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // Setup window
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = g_szClassName;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowExA(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "Activity Manager - Eisenhower Matrix",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 550,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBoxA(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Loop pesan GUI
    MSG Msg;
    while (GetMessage(&Msg, NULL, 0, 0) > 0) {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return Msg.wParam;
}
