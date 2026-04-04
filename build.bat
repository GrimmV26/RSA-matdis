@echo off
echo Membangun Aplikasi GUI RSA Kriptografi...
g++ c:\Users\Arga\OneDrive\Documents\rsa\RSA.cpp -o c:\Users\Arga\OneDrive\Documents\rsa\rsa_app.exe -mwindows -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo Gagal membangun aplikasi! Silakan periksa kembali kodenya.
    pause
    exit /b %errorlevel%
)
echo Sukses! Menjalankan rsa_app.exe...
start c:\Users\Arga\OneDrive\Documents\rsa\rsa_app.exe
