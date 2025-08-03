@echo off
echo ========================================
echo *** ACTIVITY MANAGER - Coviy's MATRIX ***
echo ========================================
echo.

echo [1/3] Kompilasi activity_manager.cpp...
g++ -c activity_manager.cpp -o .\lib\activity_manager.o
if %errorlevel% neq 0 (
    echo [X] ERROR: Gagal kompilasi activity_manager.cpp
    pause
    exit /b 1
)

echo [2/3] Kompilasi main.cpp...
g++ -c main.cpp -o .\lib\main.o
if %errorlevel% neq 0 (
    echo [X] ERROR: Gagal kompilasi main.cpp
    pause
    exit /b 1
)

echo [3/3] Linking object files...
g++ .\lib\main.o .\lib\activity_manager.o -o .\lib\activity_gui.exe -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo [X] ERROR: Gagal linking
    pause
    exit /b 1
)

echo.
echo [+] Kompilasi berhasil!
echo [+] File activity_gui.exe telah dibuat
echo.
echo [>] Menjalankan program...
echo ========================================
.\lib\activity_gui.exe