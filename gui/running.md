# Cara Kompilasi GUI Activity Manager

## Kompilasi ke folder lib:

```bash
g++ -c activity_manager.cpp -o .\lib\activity_manager.o
g++ -c main.cpp -o .\lib\main.o
g++ .\lib\main.o .\lib\activity_manager.o -o .\lib\activity_gui.exe -lgdi32 -luser32
```

## Menjalankan program:

```bash
.\lib\activity_gui.exe
```

## Fitur GUI Program:

- Input field untuk nama aktivitas
- Tombol pilihan untuk menentukan tingkat kepentingan (Penting/Tidak Penting)
- Tombol pilihan untuk menentukan tingkat urgensi (Mendesak/Tidak Mendesak)
- Tombol "Tambah Aktivitas" untuk menyimpan aktivitas ke daftar
- Tombol "Tampilkan Hasil" untuk mengategorikan dan menampilkan hasil
- Area teks untuk menampilkan hasil kategorisasi Eisenhower Matrix

## Cara Menggunakan:

1. Jalankan program dengan `.\lib\activity_gui.exe`
2. Masukkan nama aktivitas di field input
3. Pilih apakah aktivitas tersebut penting atau tidak
4. Pilih apakah aktivitas tersebut mendesak atau tidak
5. Klik "Tambah Aktivitas" untuk menyimpan
6. Ulangi langkah 2-5 untuk aktivitas lainnya
7. Klik "Tampilkan Hasil" untuk melihat kategorisasi dan rekomendasi
