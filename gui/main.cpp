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
#define ID_BTN_EXPLANATION  3003

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
std::vector<Activity> displayOrderActivities; // Activities in display order for consistent indexing
int selectedActivityIndex = -1;
bool editMode = false;

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
    int hov = 20;
    int lineHeight = 20;
    
    // Draw title
    SetTextColor(hdc, RGB(0, 100, 200));
    TextOutA(hdc, x, y, "*** COVIE'S MATRIX - KATEGORISASI AKTIVITAS ***", 48);
    y += lineHeight * 2;
    
    SetTextColor(hdc, RGB(0, 0, 0));
    
    // Draw table header
    const char* lines[] = {
        "+-----+------------------------+----------+----------+----------+------------------+",
        "| No  | Nama Aktivitas         | Penting  | Mendesak | Kuadran  | Aksi             |",
        "+-----+------------------------+----------+----------+----------+------------------+"
    };
    
    for (int i = 0; i < 3; i++) {
        TextOutA(hdc, x, y, lines[i], strlen(lines[i]));
        y += lineHeight;
    }
    
    // Get all activities directly
    const auto& allActivities = manager.getAllActivities();
    
    // Clear display order and rebuild it to match what we're showing
    displayOrderActivities.clear();
    
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
    
    // Build display order: I, II, III, IV (same as drawing order)
    for (const auto& activity : quadI) displayOrderActivities.push_back(activity);
    for (const auto& activity : quadII) displayOrderActivities.push_back(activity);
    for (const auto& activity : quadIII) displayOrderActivities.push_back(activity);
    for (const auto& activity : quadIV) displayOrderActivities.push_back(activity);
    
    // Draw activities by quadrant
    int no = 1;
    
    // Helper function to draw activities
    auto drawQuadrantActivities = [&](const std::vector<Activity>& activities, const char* quadrant) {
        for (const auto& activity : activities) {
            std::string name = activity.name;
            if (name.length() > 22) name = name.substr(0, 19) + "...";
            
            char line[120];
            sprintf(line, "| %3d | %-22s | %-8s | %-8s | %-8s |",
                    no++, name.c_str(),
                    activity.isImportant ? "Ya" : "Tidak",
                    activity.isUrgent ? "Ya" : "Tidak", quadrant);
            TextOutA(hdc, x, y, line, strlen(line));
            
            // Calculate button positions within the action column
            int actionColumnStart = x + 480; // Start of action column
            int buttonWidth = 50;
            int buttonHeight = 16;
            int buttonSpacing = 5;
            
            // Draw Edit button with background and border
            RECT editRect;
            editRect.left = actionColumnStart + 2;
            editRect.top = y + 2;
            editRect.right = editRect.left + buttonWidth;
            editRect.bottom = editRect.top + buttonHeight;
            
            // Fill button background
            // HBRUSH editBrush = CreateSolidBrush(RGB(220, 255, 220)); // Light green background
            // FillRect(hdc, &editRect, editBrush);
            // DeleteObject(editBrush);
            
            // Draw button border
            // HPEN editPen = CreatePen(PS_SOLID, 1, RGB(0, 150, 0)); // Green border
            // HPEN oldPen = (HPEN)SelectObject(hdc, editPen);
            // HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            // Rectangle(hdc, editRect.left, editRect.top, editRect.right, editRect.bottom);
            
            // Draw Edit button text
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 100, 0));
            TextOutA(hdc, editRect.left + 1, editRect.top + 2, "Edit", 4);
            
            // Draw Delete button with background and border
            RECT deleteRect;
            deleteRect.left = actionColumnStart + buttonWidth + buttonSpacing + 2;
            deleteRect.top = y + 2;
            deleteRect.right = deleteRect.left + buttonWidth;
            deleteRect.bottom = deleteRect.top + buttonHeight;
            
            // Fill button background
            // HBRUSH deleteBrush = CreateSolidBrush(RGB(255, 220, 220)); // Light red background
            // FillRect(hdc, &deleteRect, deleteBrush);
            // DeleteObject(deleteBrush);
            
            // Draw button border
            // HPEN deletePen = CreatePen(PS_SOLID, 1, RGB(200, 0, 0)); // Red border
            // SelectObject(hdc, deletePen);
            // Rectangle(hdc, deleteRect.left, deleteRect.top, deleteRect.right, deleteRect.bottom);
            
            // Draw Delete button text
            SetTextColor(hdc, RGB(150, 0, 0));
            TextOutA(hdc, deleteRect.left , deleteRect.top + 2, "Delete", 6);
            
            // Complete the table row with closing border
            SetTextColor(hdc, RGB(0, 0, 0));
            TextOutA(hdc, x + 575, y, " |", 2);
            
            // Restore original pen and brush
            // SelectObject(hdc, oldPen);
            // SelectObject(hdc, oldBrush);
            // DeleteObject(editPen);
            // DeleteObject(deletePen);
            
            y += lineHeight;
        }
    };
    
    // Draw all quadrants
    drawQuadrantActivities(quadI, "I");
    drawQuadrantActivities(quadII, "II");
    drawQuadrantActivities(quadIII, "III");
    drawQuadrantActivities(quadIV, "IV");
    
    // Draw table footer
    TextOutA(hdc, x, y, "+-----+------------------------+----------+----------+----------+------------------+", 81);
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
        HWND hBtnLoadData = CreateWindowA("BUTTON", "[<] Import Data", 
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

        // Explanation button - positioned at bottom right corner
        HWND hBtnExplanation = CreateWindowA("BUTTON", "[?] Penjelasan Kuadran", 
                                            WS_CHILD | BS_PUSHBUTTON,
                                            450, 500, 170, 35, hwnd, (HMENU)ID_BTN_EXPLANATION, NULL, NULL);
        viewControls.push_back(hBtnExplanation);

        // Results area - posisikan di luar area visible secara default
        hTextResult = CreateWindowA("EDIT", "", 
                                   WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                   -1000, -1000, 480, 300, hwnd, (HMENU)ID_TEXT_RESULT, NULL, NULL);
        viewControls.push_back(hTextResult);

        // Initialize view - show only HOME
        ShowControls(addControls, false);
        ShowControls(viewControls, false);
        ShowControls(homeControls, true);
        
        // Load data once at startup - prevent duplicates
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
                if (editMode && selectedActivityIndex >= 0) {
                    // Edit existing activity
                    manager.editActivity(selectedActivityIndex, std::string(activityName), isImportant, isUrgent);
                    MessageBoxA(hwnd, "Aktivitas berhasil diperbarui!", "Sukses", MB_OK | MB_ICONINFORMATION);
                    editMode = false;
                    selectedActivityIndex = -1;
                } else {
                    // Add new activity
                    manager.addActivity(std::string(activityName), isImportant, isUrgent);
                    MessageBoxA(hwnd, "Aktivitas berhasil ditambahkan dan disimpan!", "Sukses", MB_OK | MB_ICONINFORMATION);
                }
                
                SetWindowTextA(hEditActivity, ""); // Clear input
                
                // Reset selections
                isImportant = false;
                isUrgent = false;
                SetWindowTextA(hBtnImportant, "[!] Penting");
                SetWindowTextA(hBtnNotImportant, "[-] Tidak Penting");
                SetWindowTextA(hBtnUrgent, "[!!] Mendesak");
                SetWindowTextA(hBtnNotUrgent, "[~] Tidak Mendesak");
                
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
        
        case ID_BTN_EXPLANATION: // Tombol Penjelasan Kuadran
        {
            // Get current activities and categorize them
            manager.categorizeActivities();
            const auto& allActivities = manager.getAllActivities();
            
            // Categorize activities on the fly to get real data
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
            
            // Build explanation with real data
            std::stringstream explanation;
            explanation << "*** PENJELASAN KUADRAN EISENHOWER MATRIX ***\n\n";
            
            // KUADRAN I
            explanation << "KUADRAN I - PENTING & MENDESAK (" << quadI.size() << " aktivitas)\n";
            // explanation << "- Krisis dan emergency\n";
            // explanation << "- Deadline yang sudah dekat\n";
            // explanation << "- Masalah mendesak\n";
            explanation << "-> TINDAKAN: Kerjakan SEGERA!\n";
            if (!quadI.empty()) {
                explanation << ">> AKTIVITAS ANDA:\n";
                for (size_t i = 0; i < quadI.size(); i++) {
                    explanation << "   - " << quadI[i].name << "\n";
                }
            } else {
                explanation << "> Tidak ada aktivitas mendesak saat ini!\n";
            }
            explanation << "\n";
            
            // KUADRAN II
            explanation << "KUADRAN II - PENTING & TIDAK MENDESAK (" << quadII.size() << " aktivitas)\n";
            // explanation << "- Perencanaan dan strategi\n";
            // explanation << "- Pengembangan diri\n";
            // explanation << "- Pencegahan masalah\n";
            explanation << "-> TINDAKAN: JADWALKAN dengan baik\n";
            if (!quadII.empty()) {
                explanation << ">> AKTIVITAS ANDA:\n";
                for (size_t i = 0; i < quadII.size(); i++) {
                    explanation << "   - " << quadII[i].name << "\n";
                }
            } else {
                explanation << "!! Tambahkan aktivitas perencanaan di sini !!\n";
            }
            explanation << "\n";
            
            // KUADRAN III
            explanation << "KUADRAN III - TIDAK PENTING & MENDESAK (" << quadIII.size() << " aktivitas)\n";
            // explanation << "- Interupsi dan gangguan\n";
            // explanation << "- Meeting tidak penting\n";
            // explanation << "- Permintaan orang lain\n";
            explanation << "-> TINDAKAN: DELEGASIKAN atau TOLAK\n";
            if (!quadIII.empty()) {
                explanation << ">> AKTIVITAS ANDA:\n";
                for (size_t i = 0; i < quadIII.size(); i++) {
                    explanation << "   - " << quadIII[i].name << "\n";
                }
            } else {
                explanation << "> Bagus! Hindari aktivitas yang mengganggu!\n";
            }
            explanation << "\n";
            
            // KUADRAN IV
            explanation << "KUADRAN IV - TIDAK PENTING & TIDAK MENDESAK (" << quadIV.size() << " aktivitas)\n";
            // explanation << "- Aktivitas mengisi waktu\n";
            // explanation << "- Social media berlebihan\n";
            // explanation << "- Hiburan yang tidak produktif\n";
            explanation << "-> TINDAKAN: ELIMINASI atau MINIMALKAN\n";
            if (!quadIV.empty()) {
                explanation << ">> AKTIVITAS ANDA:\n";
                for (size_t i = 0; i < quadIV.size(); i++) {
                    explanation << "   - " << quadIV[i].name << "\n";
                }
            } else {
                explanation << "> Sempurna! Tidak ada waktu terbuang!\n";
            }
            explanation << "\n";
            
            // Tips and summary
            explanation << "!! TIPS SUKSES:\n";
            explanation << "- Fokus 60-70% waktu di Kuadran II\n";
            explanation << "- Minimalkan Kuadran III dan IV\n";
            explanation << "- Selesaikan Kuadran I secepat mungkin\n";
            explanation << "- Kuadran II adalah kunci produktivitas!\n\n";
            
            explanation << "!! RINGKASAN ANDA:\n";
            explanation << "Total: " << allActivities.size() << " aktivitas\n";
            explanation << "I: " << quadI.size() << " | II: " << quadII.size() 
                       << " | III: " << quadIII.size() << " | IV: " << quadIV.size();
            
            MessageBoxA(hwnd, explanation.str().c_str(), "Penjelasan & Data Kuadran Anda", 
                       MB_OK | MB_ICONINFORMATION);
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
    
    case WM_LBUTTONDOWN:
    {
        if (currentView == ViewMode::VIEW && (drawCustomTable || drawSearchResults)) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // Debug: Show click coordinates
            char debugMsg[100];
            sprintf(debugMsg, "Click at: x=%d, y=%d", x, y);
            // Uncomment line below for debugging coordinates
            // MessageBoxA(hwnd, debugMsg, "Debug Click", MB_OK);
            
            // Check if click is in table area 
            // Data starts at y=200 (title=100, +40 for title space, +60 for 3 header lines)
            if (x >= 20 && x <= 700 && y >= 200) {
                int rowIndex = (y - 200) / 20; // Correct calculation based on actual data start position
                int totalActivities = drawSearchResults ? currentSearchResults.size() : displayOrderActivities.size();
                
                if (rowIndex >= 0 && rowIndex < totalActivities) {
                    // Use same coordinates as in drawing function
                    int tableX = 20; // Same as x in DrawTableDirect
                    int actionColumnStart = tableX + 480; // Same as in drawing function
                    int editButtonStart = actionColumnStart + 5;
                    int editButtonEnd = editButtonStart + 35;
                    int deleteButtonStart = actionColumnStart + 35 + 5 + 10;
                    int deleteButtonEnd = deleteButtonStart + 35;
                    
                    if (x >= editButtonStart && x <= editButtonEnd) {
                        // Edit button clicked - find the real index in original data
                        const Activity& activity = drawSearchResults ? 
                            currentSearchResults[rowIndex] : 
                            displayOrderActivities[rowIndex];
                        
                        // Find the original index in allActivities
                        const auto& allActivities = manager.getAllActivities();
                        selectedActivityIndex = -1;
                        for (size_t i = 0; i < allActivities.size(); i++) {
                            const auto& act = allActivities[i];
                            if (act.name == activity.name && 
                                act.isImportant == activity.isImportant && 
                                act.isUrgent == activity.isUrgent) {
                                selectedActivityIndex = i;
                                break;
                            }
                        }
                        
                        editMode = true;
                        
                        // Set form data for editing
                        SetWindowTextA(hEditActivity, activity.name.c_str());
                        isImportant = activity.isImportant;
                        isUrgent = activity.isUrgent;
                        
                        // Update button states
                        SetWindowTextA(hBtnImportant, isImportant ? "[!] Penting [DIPILIH]" : "[!] Penting");
                        SetWindowTextA(hBtnNotImportant, !isImportant ? "[-] Tidak Penting [DIPILIH]" : "[-] Tidak Penting");
                        SetWindowTextA(hBtnUrgent, isUrgent ? "[!!] Mendesak [DIPILIH]" : "[!!] Mendesak");
                        SetWindowTextA(hBtnNotUrgent, !isUrgent ? "[~] Tidak Mendesak [DIPILIH]" : "[~] Tidak Mendesak");
                        
                        // Switch to ADD view for editing
                        MoveWindow(hTextResult, 40, 140, 480, 300, TRUE);
                        SwitchView(ViewMode::ADD);
                        InvalidateRect(hwnd, NULL, TRUE);
                        
                    } else if (x >= deleteButtonStart && x <= deleteButtonEnd) {
                        // Delete button clicked - optimized processing
                        const Activity& activity = drawSearchResults ? 
                            currentSearchResults[rowIndex] : 
                            displayOrderActivities[rowIndex];
                        
                        char confirmMsg[150];
                        sprintf(confirmMsg, "Hapus aktivitas:\n\"%s\"?", activity.name.c_str());
                        int result = MessageBoxA(hwnd, confirmMsg, "Konfirmasi Delete", MB_YESNO | MB_ICONQUESTION);
                        
                        if (result == IDYES) {
                            // Find the original index in allActivities for the selected activity
                            const auto& allActivities = manager.getAllActivities();
                            for (size_t i = 0; i < allActivities.size(); i++) {
                                const auto& act = allActivities[i];
                                if (act.name == activity.name && 
                                    act.isImportant == activity.isImportant && 
                                    act.isUrgent == activity.isUrgent) {
                                    manager.removeActivity(i);
                                    
                                    if (drawSearchResults) {
                                        // Update search results immediately
                                        currentSearchResults = manager.searchActivities(currentSearchKeyword);
                                    } else {
                                        // Recategorize activities for display
                                        manager.categorizeActivities();
                                    }
                                    break;
                                }
                            }
                            
                            // Single efficient repaint
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    }
                }
            }
        }
    }
    break;
    // HOVER
    // case WM_MOUSEMOVE:
    // {
    //     if (currentView == ViewMode::VIEW && (drawCustomTable || drawSearchResults)) {
    //         int x = LOWORD(lParam);
    //         int y = HIWORD(lParam);
            
    //         int tableX = 20; // Same as x in DrawTableDirect
    //         int actionColumnStart = tableX + 480; // Same as in drawing function
    //         int editButtonStart = actionColumnStart + 0;
    //         int editButtonEnd = editButtonStart + 35;
    //         int deleteButtonStart = actionColumnStart + 35 + 5 + 10;
    //         int deleteButtonEnd = deleteButtonStart + 35;
            
    //         if ((x >= editButtonStart && x <= editButtonEnd) || (x >= deleteButtonStart && x <= deleteButtonEnd)) {
    //             if (y >= 200) { // Same as in WM_LBUTTONDOWN
    //                 int rowIndex = (y - 200) / 20; // Same calculation as in WM_LBUTTONDOWN
    //                 int totalActivities = drawSearchResults ? currentSearchResults.size() : displayOrderActivities.size();
                    
    //                 if (rowIndex >= 0 && rowIndex < totalActivities) {
    //                     SetCursor(LoadCursor(NULL, IDC_HAND));
                        
    //                     // Highlight the button area being hovered
    //                     HDC hdc = GetDC(hwnd);
    //                     RECT buttonRect;
                        
    //                     if (x >= editButtonStart && x <= editButtonEnd) {
    //                         // Highlight Edit button with brighter background
    //                         buttonRect.left = editButtonStart;
    //                         buttonRect.top = 200 + (rowIndex * 20) + 2; // Correct calculation
    //                         buttonRect.right = editButtonEnd;
    //                         buttonRect.bottom = buttonRect.top + 16;
                            
    //                         // HBRUSH hoverBrush = CreateSolidBrush(RGB(180, 255, 180)); // Brighter green
    //                         // FillRect(hdc, &buttonRect, hoverBrush);
    //                         // DeleteObject(hoverBrush);
                            
    //                         // Draw border
    //                         // HPEN hoverPen = CreatePen(PS_SOLID, 2, RGB(0, 120, 0)); // Thicker border
    //                         // HPEN oldPen = (HPEN)SelectObject(hdc, hoverPen);
    //                         // HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    //                         // Rectangle(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom);
    //                         // SelectObject(hdc, oldPen);
    //                         // SelectObject(hdc, oldBrush);
    //                         // DeleteObject(hoverPen);
                            
    //                         // Redraw the text
    //                         // SetBkMode(hdc, TRANSPARENT);
    //                         // SetTextColor(hdc, RGB(0, 80, 0));
    //                         TextOutA(hdc, buttonRect.left + 8, buttonRect.top + 2, "", 4);
                            
    //                     } else if (x >= deleteButtonStart && x <= deleteButtonEnd) {
    //                         // Highlight Delete button with brighter background
    //                         buttonRect.left = deleteButtonStart;
    //                         buttonRect.top = 200 + (rowIndex * 20) + 2; // Correct calculation
    //                         buttonRect.right = deleteButtonEnd;
    //                         buttonRect.bottom = buttonRect.top + 16;
                            
    //                         // HBRUSH hoverBrush = CreateSolidBrush(RGB(255, 180, 180)); // Brighter red
    //                         // FillRect(hdc, &buttonRect, hoverBrush);
    //                         // DeleteObject(hoverBrush);
                            
    //                         // // Draw border
    //                         // HPEN hoverPen = CreatePen(PS_SOLID, 2, RGB(180, 0, 0)); // Thicker border
    //                         // HPEN oldPen = (HPEN)SelectObject(hdc, hoverPen);
    //                         // HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    //                         // Rectangle(hdc, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom);
    //                         // SelectObject(hdc, oldPen);
    //                         // SelectObject(hdc, oldBrush);
    //                         // DeleteObject(hoverPen);
                            
    //                         // // Redraw the text
    //                         // SetBkMode(hdc, TRANSPARENT);
    //                         // SetTextColor(hdc, RGB(120, 0, 0));
    //                         TextOutA(hdc, buttonRect.left + 5, buttonRect.top + 2, "", 6);
    //                     }
                        
    //                     ReleaseDC(hwnd, hdc);
    //                 } else {
    //                     SetCursor(LoadCursor(NULL, IDC_ARROW));
    //                 }
    //             } else {
    //                 SetCursor(LoadCursor(NULL, IDC_ARROW));
    //             }
    //         } else {
    //             SetCursor(LoadCursor(NULL, IDC_ARROW));
    //         }
    //     }
    // }
    // break;
    // END OF HOVER

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
    // Setup window first, data will be loaded in WM_CREATE
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
