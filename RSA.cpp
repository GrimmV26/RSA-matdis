#include <windows.h>
#include <string>
#include <cstdio>
#include <vector>
#include "rsa_logic.h"

// Warna Tema Dark Mode Modern
#define BG_COLOR        RGB(32, 32, 32)
#define CONTROL_BG      RGB(45, 45, 48)
#define TEXT_COLOR      RGB(255, 255, 255)
#define BTN_BG          RGB(0, 110, 200)
#define BTN_HOVER       RGB(30, 144, 255)
#define BTN_DOWN        RGB(0, 80, 160)
#define BORDER_COLOR    RGB(0, 110, 200)

#define ID_BTN_ENCRYPT  102
#define ID_BTN_DECRYPT  103
#define ID_BTN_START    110
#define ID_BTN_ABOUT    111
#define ID_BTN_EXIT     112
#define ID_BTN_BACK     113
#define ID_BTN_HITUNG   119
#define ID_CHK_HIDE_D   120
#define ID_BTN_BUAT_D   121
#define ID_EDT_CARA_ENC 124
#define ID_EDT_CARA_DEC 125

#define ID_EDT_P        116
#define ID_EDT_Q        117
#define ID_EDT_M        118
#define ID_EDT_E        104
#define ID_EDT_D        105
#define ID_EDT_N        114
#define ID_EDT_IN_ENC   106
#define ID_EDT_OUT_ENC  107
#define ID_EDT_OUT_ASCII 122
#define ID_EDT_IN_DEC   108
#define ID_EDT_OUT_DEC  109

#include "rsa_ui.h"
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        g_scrollY = 0;
        CreateUI(hWnd);
        SwitchView(hWnd, true);
        break;

    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        
        int oldScrollY = g_scrollY;
        ResizeLayout(width, height);
        UpdateScrollState(hWnd);
        if (g_scrollY != oldScrollY) {
            ResizeLayout(width, height);
        }
        break;
    }
    
    case WM_VSCROLL: {
        SCROLLINFO si;
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hWnd, SB_VERT, &si);
        int oldPos = si.nPos;

        switch (LOWORD(wParam)) {
            case SB_TOP: si.nPos = si.nMin; break;
            case SB_BOTTOM: si.nPos = si.nMax; break;
            case SB_LINEUP: si.nPos -= 30; break;
            case SB_LINEDOWN: si.nPos += 30; break;
            case SB_PAGEUP: si.nPos -= si.nPage; break;
            case SB_PAGEDOWN: si.nPos += si.nPage; break;
            case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
        }

        si.fMask = SIF_POS;
        SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hWnd, SB_VERT, &si); // Dapatkan clamped position
        
        if (si.nPos != oldPos) {
            int dy = oldPos - si.nPos;
            ScrollWindowEx(hWnd, 0, dy, NULL, NULL, NULL, NULL, SW_ERASE | SW_INVALIDATE | SW_SCROLLCHILDREN);
            g_scrollY = si.nPos;
            UpdateWindow(hWnd);
        }
        break;
    }

    case WM_MOUSEWHEEL: {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        // Scroll 2 kali setiap takik agar lebih responsif
        SendMessage(hWnd, WM_VSCROLL, zDelta > 0 ? MAKEWPARAM(SB_LINEUP, 0) : MAKEWPARAM(SB_LINEDOWN, 0), 0);
        SendMessage(hWnd, WM_VSCROLL, zDelta > 0 ? MAKEWPARAM(SB_LINEUP, 0) : MAKEWPARAM(SB_LINEDOWN, 0), 0);
        return 0;
    }

    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
        if (pdis->CtlType == ODT_BUTTON) {
            DrawModernButton(pdis);
            return TRUE;
        }
        break;
    }
        
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        HWND hCtrl = (HWND)lParam;
        char className[256];
        GetClassNameA(hCtrl, className, sizeof(className));

        SetTextColor(hdc, TEXT_COLOR);
        if (strcmp(className, "Edit") == 0) {
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, CONTROL_BG);
            return (LRESULT)hControlBrush;
        } else {
            SetBkMode(hdc, TRANSPARENT);
            SetBkColor(hdc, BG_COLOR);
            return (LRESULT)hbgBrush;
        }
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
            case ID_CHK_HIDE_D: {
                bool isChecked = SendMessage(hChkHideD, BM_GETCHECK, 0, 0) == BST_CHECKED;
                if (isChecked) {
                    SendMessage(hEdtD, EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
                } else {
                    SendMessage(hEdtD, EM_SETPASSWORDCHAR, 0, 0);
                }
                InvalidateRect(hEdtD, NULL, TRUE);
                break;
            }
            case ID_BTN_BUAT_D: {
                char eBuf[64], mBuf[64];
                GetWindowTextA(hEdtE, eBuf, sizeof(eBuf));
                GetWindowTextA(hEdtM, mBuf, sizeof(mBuf));
                
                if (strlen(eBuf) > 0 && strlen(mBuf) > 0) {
                    uint64 e = strtoull(eBuf, NULL, 10);
                    uint64 m = strtoull(mBuf, NULL, 10);
                    
                    if (gcd(e, m) != 1) {
                        MessageBoxA(hWnd, "Nilai e (Kunci Publik) tidak relatif prima dengan Totient (m)!", "Peringatan Kunci Publik", MB_ICONERROR | MB_OK);
                        SetWindowTextA(hEdtD, "");
                    } else {
                        uint64 d = modInverse(e, m);
                        char dBuf[64];
                        sprintf(dBuf, "%llu", d);
                        SetWindowTextA(hEdtD, dBuf);
                    }
                } else {
                    MessageBoxA(hWnd, "Pastikan nilai Totient (m) sudah dihitung dan Kunci Publik (e) sudah diisi!", "Peringatan", MB_ICONWARNING | MB_OK);
                }
                break;
            }
            case ID_BTN_START: {
                SwitchView(hWnd, false);
                break;
            }
            case ID_BTN_ABOUT: {
                MessageBoxA(hWnd, 
                    "Dibuat oleh kelompok 2 matematika diskrit Arga Pratama Raksajiwa", 
                    "Tentang Aplikasi", MB_ICONINFORMATION | MB_OK);
                break;
            }
            case ID_BTN_EXIT: {
                PostQuitMessage(0);
                break;
            }
            case ID_BTN_BACK: {
                SwitchView(hWnd, true);
                break;
            }
            case ID_BTN_HITUNG: {
                char pBuf[64], qBuf[64];
                GetWindowTextA(hEdtP, pBuf, sizeof(pBuf));
                GetWindowTextA(hEdtQ, qBuf, sizeof(qBuf));
                
                if (strlen(pBuf) > 0 && strlen(qBuf) > 0) {
                    uint64 p = strtoull(pBuf, NULL, 10);
                    uint64 q = strtoull(qBuf, NULL, 10);
                    
                    bool pPrime = isPrime(p);
                    bool qPrime = isPrime(q);
                    
                    if (!pPrime || !qPrime) {
                        MessageBoxA(hWnd, "Bilangan bukan prima! Mohon pastikan p dan q adalah bilangan prima murni.", "Peringatan", MB_ICONERROR | MB_OK);
                        if (!pPrime) SetWindowTextA(hEdtP, "");
                        if (!qPrime) SetWindowTextA(hEdtQ, "");
                        SetWindowTextA(hEdtN, "");
                        SetWindowTextA(hEdtM, "");
                        SetWindowTextA(hEdtE, "");
                        SetWindowTextA(hEdtD, "");
                    } else if (p == q) {
                        MessageBoxA(hWnd, "p dan q tidak boleh sama! Harap sesuaikan.", "Peringatan", MB_ICONWARNING | MB_OK);
                        SetWindowTextA(hEdtQ, "");
                    } else {
                        uint64 n = p * q;
                        uint64 m = (p - 1) * (q - 1);
                        char nBuf[64], mBuf[64];
                        sprintf(nBuf, "%llu", n);
                        sprintf(mBuf, "%llu", m);
                        SetWindowTextA(hEdtN, nBuf);
                        SetWindowTextA(hEdtM, mBuf);
                        
                        SetWindowTextA(hEdtE, "");
                        SetWindowTextA(hEdtD, "");
                    }
                } else {
                    MessageBoxA(hWnd, "Mohon isi kotak input Bilangan Prima (p) dan (q) terlebih dahulu!", "Data Kosong", MB_ICONWARNING | MB_OK);
                }
                break;
            }
            case ID_BTN_ENCRYPT: {
                char eBuf[256], nBuf[256];
                GetWindowTextA(hEdtE, eBuf, sizeof(eBuf));
                GetWindowTextA(hEdtN, nBuf, sizeof(nBuf));
                uint64 input_e = strtoull(eBuf, NULL, 10);
                uint64 input_n = strtoull(nBuf, NULL, 10);

                if (input_e == 0 || input_n == 0) {
                    MessageBoxA(hWnd, "Mohon lengkapi seluruh langkah pembuatan kunci di atas secara benar sebelum mengenkripsi!", "Kesalahan Data", MB_ICONWARNING | MB_OK);
                    break;
                }
                char inText[4096];
                GetWindowTextA(hEdtInEnc, inText, sizeof(inText));
                if (strlen(inText) == 0) {
                    MessageBoxA(hWnd, "Pesan tidak boleh kosong!", "Peringatan", MB_ICONWARNING | MB_OK);
                    break;
                }
                
                std::string msg(inText);
                std::string asciiStr = "";
                std::vector<uint64> encrypted;
                std::string stepStr = "Rumus Enkripsi C = P^e mod n\r\n(Di mana e=" + std::string(eBuf) + ", n=" + std::string(nBuf) + ")\r\n--------------------------------\r\n";
                
                // Iterasi Karakter Murni dengan Padding 3-Digit
                for (size_t i = 0; i < msg.length(); i++) {
                    unsigned char c = (unsigned char)msg[i];
                    char buf[16];
                    sprintf(buf, "%03d", c);
                    
                    if (i != 0) asciiStr += " ";
                    asciiStr += buf;
                    
                    uint64 m_val = (uint64)c;
                    uint64 c_val = modPow(m_val, input_e, input_n);
                    encrypted.push_back(c_val);
                    
                    char stepBuf[512];
                    sprintf(stepBuf, "1. Baca karakter teks: '%c'\r\n2. Konversi karakter '%c' menjadi nilai desimal ASCII: %llu\r\n3. Hitung Enkripsi (C) = %llu^%llu mod %llu = %llu (Cipherteks)\r\n--------------------------------\r\n", c, c, m_val, m_val, input_e, input_n, c_val);
                    stepStr += stepBuf;
                }
                
                std::string cipherStr = cipherToString(encrypted);
                
                SetWindowTextA(hEdtOutAscii, asciiStr.c_str());
                SetWindowTextA(hEdtOutEnc, cipherStr.c_str());
                SetWindowTextA(hEdtInDec, cipherStr.c_str());
                SetWindowTextA(hEdtCaraEnc, stepStr.c_str());
                break;
            }
            case ID_BTN_DECRYPT: {
                char dBuf[256], nBuf[256];
                GetWindowTextA(hEdtD, dBuf, sizeof(dBuf));
                GetWindowTextA(hEdtN, nBuf, sizeof(nBuf));
                uint64 input_d = strtoull(dBuf, NULL, 10);
                uint64 input_n = strtoull(nBuf, NULL, 10);

                if (input_d == 0 || input_n == 0) {
                    MessageBoxA(hWnd, "Mohon pastikan Kunci Dekripsi (d) dan Modulus (n) sudah terpenuhi di kolom kunci!", "Kesalahan Data", MB_ICONWARNING | MB_OK);
                    break;
                }
                char inCipher[8192];
                GetWindowTextA(hEdtInDec, inCipher, sizeof(inCipher));
                if (strlen(inCipher) == 0) {
                    MessageBoxA(hWnd, "Ciphertext tidak boleh kosong!", "Peringatan", MB_ICONWARNING | MB_OK);
                    break;
                }
                
                std::string cipherStr(inCipher);
                std::vector<uint64> encrypted = stringToCipher(cipherStr);
                
                std::string decryptedMsg = "";
                std::string stepStr = "Rumus Dekripsi P = C^d mod n\r\n(Di mana d=" + std::string(dBuf) + ", n=" + std::string(nBuf) + ")\r\n--------------------------------\r\n";
                
                // Dekripsi satu per satu cipher dari array ciphertext, tak perlu peduli panjang digit awalnya
                for (size_t i = 0; i < encrypted.size(); ++i) {
                    uint64 decVal = modPow(encrypted[i], input_d, input_n);
                    char charRes = '?';
                    
                    // Batas proteksi overflow (meski karakter ASCII murni takkan lebih dari 255)
                    if (decVal <= 255) {
                        charRes = (char)decVal;
                        decryptedMsg += charRes;
                    } else {
                        decryptedMsg += charRes; // Karakter rusak penanda modulus terlampaui/salah kunci
                    }
                    
                    char stepBuf[512];
                    sprintf(stepBuf, "1. Baca blok cipher: %llu\r\n2. Hitung Dekripsi (P) = %llu^%llu mod %llu = %llu (Nilai ASCII Asli)\r\n3. Kembalikan nilai letak ASCII %llu tersebut menjadi karakter teks: '%c'\r\n--------------------------------\r\n", encrypted[i], encrypted[i], input_d, input_n, decVal, decVal, charRes);
                    stepStr += stepBuf;
                }
                
                SetWindowTextA(hEdtOutDec, decryptedMsg.c_str());
                SetWindowTextA(hEdtCaraDec, stepStr.c_str());
                break;
            }
        }
        break;
    }
    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 960;
        lpMMI->ptMinTrackSize.y = 540; 
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    hbgBrush = CreateSolidBrush(BG_COLOR);
    hControlBrush = CreateSolidBrush(CONTROL_BG);
    wcex.hbrBackground = hbgBrush;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "RsaModernAppClass";
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(1));

    if(!RegisterClassEx(&wcex)) {
        return FALSE;
    }

    hFont = CreateFontA(18, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    hBigTitleFont = CreateFontA(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

    // Menambahkan WS_VSCROLL pada jendela utama
    HWND hWnd = CreateWindow(
        "RsaModernAppClass", "Kriptografi RSA - Responsive", 
        WS_OVERLAPPEDWINDOW | WS_MAXIMIZEBOX | WS_VSCROLL, 
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 540, 
        NULL, NULL, hInstance, NULL);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(hbgBrush);
    DeleteObject(hControlBrush);
    DeleteObject(hFont);
    DeleteObject(hTitleFont);
    DeleteObject(hBigTitleFont);
    return (int) msg.wParam;
}

