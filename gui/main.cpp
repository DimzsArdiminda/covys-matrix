#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

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
#define ID_BTN_LOAD_DATA    1009
#define ID_BTN_CLEAR_DATA   1010
#define ID_BTN_HOME         1011
#define ID_BTN_TO_ADD       2001
#define ID_BTN_TO_VIEW      2002
#define ID_EDIT_SEARCH      3001

// Global variables
ActivityManager manager;
HWND hEditActivity, hBtnImportant, hBtnNotImportant, hBtnUrgent, hBtnNotUrgent;
HWND hBtnAdd, hBtnShowResult, hTextResult, hBtnLoadData, hBtnClearData;
HWND hEditSearch, hBtnHome;
bool isImportant = false, isUrgent = false;
bool drawCustomTable = false;
bool drawSearchResults = false;
std::vector<Activity> currentSearchResults;
std::string currentSearchKeyword;

// View management
enum class ViewMode { HOME, ADD, VIEW };
ViewMode currentView = ViewMode::HOME;
std::vector<HWND> homeControls, addControls, viewControls;

// Fungsi utilitas
void ShowControls(const std::vector<HWND>& controls, bool show) {
    for (auto h : controls) ShowWindow(h, show ? SW_SHOW : SW_HIDE);
}

void SwitchView(ViewMode mode) {
    currentView = mode;
    ShowControls(homeControls, mode == ViewMode::HOME);
    ShowControls(addControls, mode == ViewMode::ADD);
    ShowControls(viewControls, mode == ViewMode::VIEW);
}

// Fungsi untuk update display otomatis
void UpdateViewDisplay(HWND hwnd) {
    if (currentView == ViewMode::VIEW) {
        manager.categorizeActivities();
        drawCustomTable = true;
        ShowWindow(hTextResult, SW_HIDE);
        InvalidateRect(hwnd, NULL, TRUE); // Force repaint
    }
}

// Fungsi untuk menggambar tabel langsung di window
void DrawTableDirect(HDC hdc, HWND hwnd) {
    if (!drawCustomTable) return;
    
    // Setup font
    HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    
    int x = 20;
    int y = 100;
    int lineHeight = 20;
    
    // Draw title
    SetTextColor(hdc, RGB(0, 100, 200));
    TextOutA(hdc, x, y, "*** COVIE'S MATRIX - KATEGORISASI AKTIVITAS ***", 48);
    y += lineHeight * 2;
    
    SetTextColor(hdc, RGB(0, 0, 0));
    
    // Draw table header
    const char* lines[] = {
        "+-----+--------------------------------+----------+----------+----------+",
        "| No  | Nama Aktivitas                 | Penting  | Mendesak | Kuadran  |",
        "+-----+--------------------------------+----------+----------+----------+"
    };
    
    for (int i = 0; i < 3; i++) {
        TextOutA(hdc, x, y, lines[i], strlen(lines[i]));
        y += lineHeight;
    }
    
    // Get all activities directly
    const auto& allActivities = manager.getAllActivities();
    
    // Categorize activities on the fly
    std::vector<Activity> quadI, quadII, quadIII, quadIV;
    
    for (const auto& activity : allActivities) {
        if (activity.isImportant && activity.isUrgent) {
            quadI.push_back(activity);
        } else if (activity.isImportant && !activity.isUrgent) {
            quadII.push_back(activity);
        } else if (!activity.isImportant && activity.isUrgent) {
            quadIII.push_back(activity);
        } else {
            quadIV.push_back(activity);
        }
    }
    
    // Draw activities by quadrant
    int no = 1;
    
    // Helper function to draw activities
    auto drawQuadrantActivities = [&](const std::vector<Activity>& activities, const char* quadrant) {
        for (const auto& activity : activities) {
            std::string name = activity.name;
            if (name.length() > 30) name = name.substr(0, 27) + "...";
            
            char line[100];
            sprintf(line, "| %3d | %-30s | %-8s | %-8s | %-8s |",
                    no++, name.c_str(),
                    activity.isImportant ? "Ya" : "Tidak",
                    activity.isUrgent ? "Ya" : "Tidak", quadrant);
            TextOutA(hdc, x, y, line, strlen(line));
            y += lineHeight;
        }
    };
    
    // Draw all quadrants
    drawQuadrantActivities(quadI, "I");
    drawQuadrantActivities(quadII, "II");
    drawQuadrantActivities(quadIII, "III");
    drawQuadrantActivities(quadIV, "IV");
    
    // Draw table footer
    TextOutA(hdc, x, y, "+-----+--------------------------------+----------+----------+----------+", 69);
    y += lineHeight * 2;
    
    // Draw summary
    SetTextColor(hdc, RGB(0, 150, 0));
    char summary[200];
    sprintf(summary, "Ringkasan: [I] %d  [II] %d  [III] %d  [IV] %d  Total: %d aktivitas",
            (int)quadI.size(), (int)quadII.size(), (int)quadIII.size(), (int)quadIV.size(),
            (int)allActivities.size());
    TextOutA(hdc, x, y, summary, strlen(summary));
    
    // Cleanup
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// Fungsi untuk menggambar hasil pencarian dalam format tabel
void DrawSearchResults(HDC hdc, HWND hwnd) {
    if (!drawSearchResults) return;
    
    // Setup font
    HFONT hFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    
    int x = 20;
    int y = 100;
    int lineHeight = 20;
    
    // Draw search title
    SetTextColor(hdc, RGB(200, 0, 0));
    char searchTitle[100];
    sprintf(searchTitle, "*** HASIL PENCARIAN: \"%s\" ***", currentSearchKeyword.c_str());
    TextOutA(hdc, x, y, searchTitle, strlen(searchTitle));
    y += lineHeight * 2;
    
    SetTextColor(hdc, RGB(0, 0, 0));
    
    if (currentSearchResults.empty()) {
        TextOutA(hdc, x, y, "[!] Tidak ada aktivitas yang ditemukan.", 36);
        y += lineHeight;
        TextOutA(hdc, x, y, "    Coba gunakan kata kunci yang berbeda.", 41);
    } else {
        char foundText[50];
        sprintf(foundText, "[+] Ditemukan %d aktivitas:", (int)currentSearchResults.size());
        TextOutA(hdc, x, y, foundText, strlen(foundText));
        y += lineHeight * 2;
        
        // Draw table header
        const char* lines[] = {
            "+-----+--------------------------------+----------+----------+----------+",
            "| No  | Nama Aktivitas                 | Penting  | Mendesak | Kuadran  |",
            "+-----+--------------------------------+----------+----------+----------+"
        };
        
        for (int i = 0; i < 3; i++) {
            TextOutA(hdc, x, y, lines[i], strlen(lines[i]));
            y += lineHeight;
        }
        
        // Draw search results
        int no = 1;
        for (const auto& activity : currentSearchResults) {
            std::string name = activity.name;
            if (name.length() > 30) name = name.substr(0, 27) + "...";
            
            // Determine quadrant
            const char* quadrant;
            if (activity.isImportant && activity.isUrgent) {
                quadrant = "I";
            } else if (activity.isImportant && !activity.isUrgent) {
                quadrant = "II";
            } else if (!activity.isImportant && activity.isUrgent) {
                quadrant = "III";
            } else {
                quadrant = "IV";
            }
            
            char line[100];
            sprintf(line, "| %3d | %-30s | %-8s | %-8s | %-8s |",
                    no++, name.c_str(),
                    activity.isImportant ? "Ya" : "Tidak",
                    activity.isUrgent ? "Ya" : "Tidak", quadrant);
            TextOutA(hdc, x, y, line, strlen(line));
            y += lineHeight;
        }
        
        // Draw table footer
        TextOutA(hdc, x, y, "+-----+--------------------------------+----------+----------+----------+", 69);
    }
    
    // Cleanup
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}


// Window Proc
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {   
        // ======= HOME VIEW =======
        // Title dengan font yang lebih besar (simulasi)
        HWND hHomeTitle = CreateWindowA("STATIC", "*** ACTIVITY MANAGER ***", 
                                        WS_CHILD | SS_CENTER,
                                        150, 50, 300, 40, hwnd, NULL, NULL, NULL);
        homeControls.push_back(hHomeTitle);

        // Subtitle
        HWND hSubtitle = CreateWindowA("STATIC", "Kelola aktivitas Anda dengan Covie's Matrix", 
                                      WS_CHILD | SS_CENTER,
                                      100, 90, 400, 25, hwnd, NULL, NULL, NULL);
        homeControls.push_back(hSubtitle);

        // Main menu buttons dengan ukuran yang lebih besar
        HWND hBtnToAdd = CreateWindowA("BUTTON", "[+] Tambah Kegiatan Baru", 
                                      WS_CHILD | BS_PUSHBUTTON,
                                      180, 140, 240, 50, hwnd, (HMENU)ID_BTN_TO_ADD, NULL, NULL);
        homeControls.push_back(hBtnToAdd);

        HWND hBtnToView = CreateWindowA("BUTTON", "[?] Tampilkan dan Kelola Kegiatan", 
                                       WS_CHILD | BS_PUSHBUTTON,
                                       180, 210, 240, 50, hwnd, (HMENU)ID_BTN_TO_VIEW, NULL, NULL);
        homeControls.push_back(hBtnToView);

        // Data management buttons di home
        HWND hBtnLoadData = CreateWindowA("BUTTON", "[<] Muat Data", 
                                         WS_CHILD | BS_PUSHBUTTON,
                                         180, 280, 115, 35, hwnd, (HMENU)ID_BTN_LOAD_DATA, NULL, NULL);
        homeControls.push_back(hBtnLoadData);
        
        HWND hBtnClearData = CreateWindowA("BUTTON", "[X] Hapus Semua", 
                                          WS_CHILD | BS_PUSHBUTTON,
                                          305, 280, 115, 35, hwnd, (HMENU)ID_BTN_CLEAR_DATA, NULL, NULL);
        homeControls.push_back(hBtnClearData);

        // ======= ADD VIEW =======
        // Header dengan tombol kembali
        hBtnHome = CreateWindowA("BUTTON", "[<] Beranda", 
                                WS_CHILD | BS_PUSHBUTTON,
                                20, 20, 100, 30, hwnd, (HMENU)ID_BTN_HOME, NULL, NULL);
        addControls.push_back(hBtnHome);

        HWND hAddTitle = CreateWindowA("STATIC", "[+] TAMBAH KEGIATAN BARU", 
                                      WS_CHILD | SS_CENTER,
                                      130, 25, 300, 30, hwnd, NULL, NULL, NULL);
        addControls.push_back(hAddTitle);

        // Input section dengan styling yang lebih baik
        HWND hLabelName = CreateWindowA("STATIC", ">> Nama Aktivitas:", 
                                       WS_CHILD,
                                       40, 80, 150, 20, hwnd, NULL, NULL, NULL);
        addControls.push_back(hLabelName);

        hEditActivity = CreateWindowA("EDIT", "", 
                                     WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                     40, 105, 480, 30, hwnd, (HMENU)ID_EDIT_ACTIVITY, NULL, NULL);
        addControls.push_back(hEditActivity);

        // Important section
        HWND hLabelImportant = CreateWindowA("STATIC", "* Tingkat Kepentingan:", 
                                            WS_CHILD,
                                            40, 155, 200, 20, hwnd, NULL, NULL, NULL);
        addControls.push_back(hLabelImportant);

        hBtnImportant = CreateWindowA("BUTTON", "[!] Penting", 
                                     WS_CHILD | BS_PUSHBUTTON,
                                     40, 180, 120, 35, hwnd, (HMENU)ID_BTN_IMPORTANT, NULL, NULL);
        addControls.push_back(hBtnImportant);

        hBtnNotImportant = CreateWindowA("BUTTON", "[-] Tidak Penting", 
                                        WS_CHILD | BS_PUSHBUTTON,
                                        170, 180, 120, 35, hwnd, (HMENU)ID_BTN_NOT_IMPORTANT, NULL, NULL);
        addControls.push_back(hBtnNotImportant);

        // Urgent section
        HWND hLabelUrgent = CreateWindowA("STATIC", "@ Tingkat Urgensi:", 
                                         WS_CHILD,
                                         40, 235, 200, 20, hwnd, NULL, NULL, NULL);
        addControls.push_back(hLabelUrgent);

        hBtnUrgent = CreateWindowA("BUTTON", "[!!] Mendesak", 
                                  WS_CHILD | BS_PUSHBUTTON,
                                  40, 260, 120, 35, hwnd, (HMENU)ID_BTN_URGENT, NULL, NULL);
        addControls.push_back(hBtnUrgent);

        hBtnNotUrgent = CreateWindowA("BUTTON", "[~] Tidak Mendesak", 
                                     WS_CHILD | BS_PUSHBUTTON,
                                     170, 260, 120, 35, hwnd, (HMENU)ID_BTN_NOT_URGENT, NULL, NULL);
        addControls.push_back(hBtnNotUrgent);

        // Save button
        hBtnAdd = CreateWindowA("BUTTON", "[SAVE] SIMPAN AKTIVITAS", 
                               WS_CHILD | BS_PUSHBUTTON,
                               40, 320, 250, 40, hwnd, (HMENU)ID_BTN_ADD, NULL, NULL);
        addControls.push_back(hBtnAdd);

        // ======= VIEW VIEW =======
        // Header dengan tombol kembali
        HWND hBtnHomeView = CreateWindowA("BUTTON", "[<] Beranda", 
                                         WS_CHILD | BS_PUSHBUTTON,
                                         20, 20, 100, 30, hwnd, (HMENU)ID_BTN_HOME, NULL, NULL);
        viewControls.push_back(hBtnHomeView);

        HWND hViewTitle = CreateWindowA("STATIC", "[?] KELOLA KEGIATAN", 
                                       WS_CHILD | SS_CENTER,
                                       130, 25, 300, 30, hwnd, NULL, NULL, NULL);
        viewControls.push_back(hViewTitle);

        // Search section
        HWND hSearchLabel = CreateWindowA("STATIC", ">> Cari aktivitas:", 
                                         WS_CHILD,
                                         40, 70, 120, 20, hwnd, NULL, NULL, NULL);
        viewControls.push_back(hSearchLabel);

        hEditSearch = CreateWindowA("EDIT", "", 
                                   WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                   40, 95, 380, 25, hwnd, (HMENU)ID_EDIT_SEARCH, NULL, NULL);
        viewControls.push_back(hEditSearch);

        HWND hBtnSearch = CreateWindowA("BUTTON", "[S] Cari", 
                                       WS_CHILD | BS_PUSHBUTTON,
                                       430, 95, 90, 25, hwnd, (HMENU)3002, NULL, NULL);
        viewControls.push_back(hBtnSearch);

        // Results area - posisikan di luar area visible secara default
        hTextResult = CreateWindowA("EDIT", "", 
                                   WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                   -1000, -1000, 480, 300, hwnd, (HMENU)ID_TEXT_RESULT, NULL, NULL);
        viewControls.push_back(hTextResult);

        // Initialize view - show only HOME
        ShowControls(addControls, false);
        ShowControls(viewControls, false);
        ShowControls(homeControls, true);
        
        // Auto-load data if available
        manager.loadFromCSV();
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case ID_BTN_TO_ADD:  // Tombol "Tambah Kegiatan"
            drawCustomTable = false;
            drawSearchResults = false;
            // Move text control back to visible area for other uses
            MoveWindow(hTextResult, 40, 140, 480, 300, TRUE);
            SwitchView(ViewMode::ADD);
            // Clear the background to remove table remnants
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case ID_BTN_TO_VIEW:  // Tombol "Tampilkan Kegiatan"
            manager.categorizeActivities();
            drawCustomTable = true;
            drawSearchResults = false;
            // Move text control out of view and show table
            MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
            SwitchView(ViewMode::VIEW);
            InvalidateRect(hwnd, NULL, TRUE); // Force repaint
            break;

        case ID_BTN_HOME:  // Tombol kembali ke beranda
            drawCustomTable = false;
            drawSearchResults = false;
            // Move text control back to visible area
            MoveWindow(hTextResult, 40, 140, 480, 300, TRUE);
            SwitchView(ViewMode::HOME);
            // Clear the background to remove table remnants
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case ID_BTN_IMPORTANT:
            isImportant = true;
            SetWindowTextA(hBtnImportant, "[!] Penting [DIPILIH]");
            SetWindowTextA(hBtnNotImportant, "[-] Tidak Penting");
            break;
            
        case ID_BTN_NOT_IMPORTANT:
            isImportant = false;
            SetWindowTextA(hBtnImportant, "[!] Penting");
            SetWindowTextA(hBtnNotImportant, "[-] Tidak Penting [DIPILIH]");
            break;
            
        case ID_BTN_URGENT:
            isUrgent = true;
            SetWindowTextA(hBtnUrgent, "[!!] Mendesak [DIPILIH]");
            SetWindowTextA(hBtnNotUrgent, "[~] Tidak Mendesak");
            break;
            
        case ID_BTN_NOT_URGENT:
            isUrgent = false;
            SetWindowTextA(hBtnUrgent, "[!!] Mendesak");
            SetWindowTextA(hBtnNotUrgent, "[~] Tidak Mendesak [DIPILIH]");
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
                SetWindowTextA(hBtnImportant, "[!] Penting");
                SetWindowTextA(hBtnNotImportant, "[-] Tidak Penting");
                SetWindowTextA(hBtnUrgent, "[!!] Mendesak");
                SetWindowTextA(hBtnNotUrgent, "[~] Tidak Mendesak");
                
                MessageBoxA(hwnd, "Aktivitas berhasil ditambahkan dan disimpan!", "Sukses", MB_OK | MB_ICONINFORMATION);
                
                // Auto-switch to view and update display
                manager.categorizeActivities();
                drawCustomTable = true;
                drawSearchResults = false;
                // Move text control out of view for custom drawing
                MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
                SwitchView(ViewMode::VIEW);
                InvalidateRect(hwnd, NULL, TRUE); // Force repaint
            } else {
                MessageBoxA(hwnd, "Mohon masukkan nama aktivitas terlebih dahulu!", "Peringatan", MB_OK | MB_ICONWARNING);
            }
        }
        break;

        case 3002: // Tombol Cari
        {
            char keyword[256];
            GetWindowTextA(hEditSearch, keyword, sizeof(keyword));
            
            if (strlen(keyword) > 0) {
                // Search and display results in table format
                currentSearchResults = manager.searchActivities(std::string(keyword));
                currentSearchKeyword = std::string(keyword);
                
                drawCustomTable = false;
                drawSearchResults = true;
                // Move text control out of view for custom drawing
                MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            } else {
                // Jika tidak ada keyword, tampilkan semua dengan custom drawing
                manager.categorizeActivities();
                drawCustomTable = true;
                drawSearchResults = false;
                // Move text control out of view for custom drawing
                MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        break;
        
        case ID_BTN_LOAD_DATA:
        {
            manager.loadFromCSV();
            MessageBoxA(hwnd, "Data berhasil dimuat dari activities.csv!", "Info", MB_OK | MB_ICONINFORMATION);
            
            // Auto-update display if we're in VIEW mode
            if (currentView == ViewMode::VIEW) {
                manager.categorizeActivities();
                drawCustomTable = true;
                drawSearchResults = false;
                // Move text control out of view for custom drawing
                MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
                InvalidateRect(hwnd, NULL, TRUE); // Force repaint
            }
        }
        break;
        
        case ID_BTN_CLEAR_DATA:
        {
            int result = MessageBoxA(hwnd, "Yakin ingin menghapus SEMUA data aktivitas?\n\nTindakan ini tidak dapat dibatalkan!", 
                                   "Konfirmasi Hapus", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES) {
                manager.clearAllActivities();
                manager.saveToCSV(); // Save empty state
                SetWindowTextA(hTextResult, "");
                MessageBoxA(hwnd, "Semua data telah dihapus!", "Info", MB_OK | MB_ICONINFORMATION);
                
                // Auto-update display if we're in VIEW mode
                if (currentView == ViewMode::VIEW) {
                    manager.categorizeActivities();
                    drawCustomTable = true;
                    drawSearchResults = false;
                    // Move text control out of view for custom drawing
                    MoveWindow(hTextResult, -1000, -1000, 480, 300, TRUE);
                    InvalidateRect(hwnd, NULL, TRUE); // Force repaint
                }
            }
        }
        break;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        // Always clear the background first
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));
        
        // Draw appropriate content based on mode
        if (currentView == ViewMode::VIEW) {
            if (drawSearchResults) {
                DrawSearchResults(hdc, hwnd);
            } else if (drawCustomTable) {
                DrawTableDirect(hdc, hwnd);
            }
        }
        
        EndPaint(hwnd, &ps);
    }
    break;
    
    case WM_ERASEBKGND:
    {
        // Handle background erasing to ensure clean transitions
        HDC hdc = (HDC)wParam;
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));
        return TRUE; // We handled the erasing
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
    // Load saved data at startup
    manager.loadFromCSV();
    
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
        "*** Activity Manager - Eisenhower Matrix ***",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 650, 600,
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
