HBRUSH hbgBrush = NULL;
HBRUSH hControlBrush = NULL;
HFONT hFont = NULL;
HFONT hTitleFont = NULL;
HFONT hBigTitleFont = NULL;

std::vector<HWND> menuControls;
std::vector<HWND> appControls;

// Handles Komponen
HWND hMenuTitle, hMenuSub, hBtnStart, hBtnAbout, hBtnExit;
HWND hAppTitle, hBtnBack;
HWND hLblP, hEdtP, hLblQ, hEdtQ, hBtnHitung;
HWND hLblHasil, hLblN, hEdtN, hLblM, hEdtM;
HWND hLblSubKunci, hChkHideD;
HWND hLblE, hEdtE, hBtnBuatD;
HWND hLblD, hEdtD;
HWND hLblEnc, hBtnEnc, hLblEncRes, hLblEncAscii;
HWND hEdtInEnc, hEdtOutAscii, hEdtOutEnc, hEdtInDec, hEdtOutDec, hLblDec, hBtnDec, hLblDecRes;

// Komponen Cara Pengerjaan
HWND hLblCaraEnc, hEdtCaraEnc;
HWND hLblCaraDec, hEdtCaraDec;

int g_scrollY = 0; // State scroll global untuk Window Utama
bool g_isAppView = false; // Membedakan antara Menu dan App untuk Scrollbar

// State animasi
struct BtnState {
    float hoverAlpha;
    bool isHovered;
};

WNDPROC oldButtonProc;
WNDPROC oldEditProc;

COLORREF BlendColor(COLORREF colorA, COLORREF colorB, float alpha) {
    int r = GetRValue(colorA) + alpha * (GetRValue(colorB) - GetRValue(colorA));
    int g = GetGValue(colorA) + alpha * (GetGValue(colorB) - GetGValue(colorA));
    int b = GetBValue(colorA) + alpha * (GetBValue(colorB) - GetBValue(colorA));
    return RGB(r, g, b);
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessage(hwnd, EM_SETSEL, 0, -1);
        return 0;
    }
    return CallWindowProc(oldEditProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ModernButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    BtnState* state = (BtnState*)GetPropA(hwnd, "BTN_STATE");
    if (!state && uMsg != WM_DESTROY) {
        state = new BtnState{0.0f, false};
        SetPropA(hwnd, "BTN_STATE", (HANDLE)state);
    }

    switch (uMsg) {
        case WM_MOUSEMOVE: {
            if (!state->isHovered) {
                state->isHovered = true;
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
                SetTimer(hwnd, 1, 15, NULL); 
            }
            break;
        }
        case WM_MOUSELEAVE: {
            if(state) state->isHovered = false;
            SetTimer(hwnd, 2, 15, NULL); 
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) { 
                if(state) state->hoverAlpha += 0.15f;
                if (state && state->hoverAlpha >= 1.0f) {
                    state->hoverAlpha = 1.0f;
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == 2) { 
                if(state) state->hoverAlpha -= 0.1f;
                if (state && state->hoverAlpha <= 0.0f) {
                    state->hoverAlpha = 0.0f;
                    KillTimer(hwnd, 2);
                }
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_DESTROY: {
            if (state) {
                delete state;
                RemovePropA(hwnd, "BTN_STATE");
            }
            break;
        }
    }
    return CallWindowProc(oldButtonProc, hwnd, uMsg, wParam, lParam);
}

// ... CreateModernBtn, CreateStdLabel, CreateStdEdit ... //

HWND CreateModernBtn(const char* text, HWND parent, int id, bool isMenu) {
    HWND btn = CreateWindow("BUTTON", text, WS_CHILD | BS_OWNERDRAW, 0, 0, 0, 0, parent, (HMENU)id, NULL, NULL);
    oldButtonProc = (WNDPROC)SetWindowLongPtr(btn, GWLP_WNDPROC, (LONG_PTR)ModernButtonProc);
    if(isMenu) menuControls.push_back(btn); else appControls.push_back(btn);
    return btn;
}

HWND CreateStdLabel(const char* text, HWND parent, HFONT font, bool isMenu, int align = 0) {
    HWND lbl = CreateWindow("STATIC", text, WS_CHILD | align, 0, 0, 0, 0, parent, NULL, NULL, NULL);
    SendMessage(lbl, WM_SETFONT, (WPARAM)font, TRUE);
    if(isMenu) menuControls.push_back(lbl); else appControls.push_back(lbl);
    return lbl;
}

HWND CreateStdEdit(const char* text, HWND parent, HFONT font, int id, DWORD styleExtra, bool isMenu) {
    HWND edt = CreateWindow("EDIT", text, WS_CHILD | WS_BORDER | styleExtra, 0, 0, 0, 0, parent, (HMENU)id, NULL, NULL);
    SendMessage(edt, WM_SETFONT, (WPARAM)font, TRUE);
    
    WNDPROC currentProc = (WNDPROC)SetWindowLongPtr(edt, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);
    if (!oldEditProc) oldEditProc = currentProc;
    
    if(isMenu) menuControls.push_back(edt); else appControls.push_back(edt);
    return edt;
}

void CreateUI(HWND hWnd) {
    // ---- MENU UI ----
    hMenuTitle = CreateStdLabel("KRIPTOGRAFI RSA", hWnd, hBigTitleFont, true, SS_CENTER);
    hMenuSub = CreateStdLabel("Program Enkripsi Algoritma RSA", hWnd, hFont, true, SS_CENTER);

    hBtnStart = CreateModernBtn("MULAI", hWnd, ID_BTN_START, true);
    hBtnAbout = CreateModernBtn("TENTANG", hWnd, ID_BTN_ABOUT, true);
    hBtnExit = CreateModernBtn("KELUAR", hWnd, ID_BTN_EXIT, true);

    // ---- APP UI ----
    hAppTitle = CreateStdLabel("Alat Kriptografi RSA", hWnd, hTitleFont, false);
    hBtnBack = CreateModernBtn("KEMBALI KE MENU", hWnd, ID_BTN_BACK, false);

    // Baris Prima dan Tombol Hitung
    hLblP = CreateStdLabel("Bilangan Prima (p):", hWnd, hFont, false);
    hEdtP = CreateStdEdit("", hWnd, hFont, ID_EDT_P, ES_AUTOHSCROLL | ES_NUMBER, false);

    hLblQ = CreateStdLabel("Bilangan Prima (q):", hWnd, hFont, false);
    hEdtQ = CreateStdEdit("", hWnd, hFont, ID_EDT_Q, ES_AUTOHSCROLL | ES_NUMBER, false);

    hBtnHitung = CreateModernBtn("HITUNG", hWnd, ID_BTN_HITUNG, false);

    // Hasil section
    hLblHasil = CreateStdLabel("Hasil Kalkulasi RSA:", hWnd, hTitleFont, false);

    hLblN = CreateStdLabel("Modulus (n = p*q):", hWnd, hFont, false);
    hEdtN = CreateStdEdit("", hWnd, hFont, ID_EDT_N, ES_AUTOHSCROLL | ES_READONLY, false);

    hLblM = CreateStdLabel("Totient (m):", hWnd, hFont, false);
    hEdtM = CreateStdEdit("", hWnd, hFont, ID_EDT_M, ES_AUTOHSCROLL | ES_READONLY, false);

    // Subjudul Kunci
    hLblSubKunci = CreateStdLabel("Struktur Kunci:", hWnd, hTitleFont, false);

    hLblE = CreateStdLabel("Kunci Publik (e):", hWnd, hFont, false);
    hEdtE = CreateStdEdit("", hWnd, hFont, ID_EDT_E, ES_AUTOHSCROLL | ES_NUMBER, false);
    
    hBtnBuatD = CreateModernBtn("BUAT KUNCI RAHASIA", hWnd, ID_BTN_BUAT_D, false);

    hLblD = CreateStdLabel("Kunci Dekripsi / Privat (d):", hWnd, hFont, false);
    
    // Checkbox custom
    hChkHideD = CreateWindow("BUTTON", "Sembunyikan", WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, (HMENU)ID_CHK_HIDE_D, NULL, NULL);
    SendMessage(hChkHideD, WM_SETFONT, (WPARAM)hFont, TRUE);
    appControls.push_back(hChkHideD);

    // D Edit dengan ES_PASSWORD default
    hEdtD = CreateStdEdit("", hWnd, hFont, ID_EDT_D, ES_AUTOHSCROLL | ES_NUMBER | ES_PASSWORD, false);
    SendMessage(hEdtD, EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
    SendMessage(hChkHideD, BM_SETCHECK, BST_CHECKED, 0);

    // Seksi Teks
    hLblEnc = CreateStdLabel("Pesan Asli (Untuk Dienkripsi):", hWnd, hTitleFont, false);
    hEdtInEnc = CreateStdEdit("HARI INI", hWnd, hFont, ID_EDT_IN_ENC, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL, false);

    hBtnEnc = CreateModernBtn("ENKRIPSI", hWnd, ID_BTN_ENCRYPT, false);
    hLblEncAscii = CreateStdLabel("Representasi Desimal ASCII:", hWnd, hFont, false);
    hEdtOutAscii = CreateStdEdit("", hWnd, hFont, ID_EDT_OUT_ASCII, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, false);
    hLblEncRes = CreateStdLabel("Hasil Enkripsi (Ciphertext):", hWnd, hFont, false);
    hEdtOutEnc = CreateStdEdit("", hWnd, hFont, ID_EDT_OUT_ENC, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, false);

    hLblDec = CreateStdLabel("Algoritma Dekripsi (Cipherteks):", hWnd, hTitleFont, false);
    hEdtInDec = CreateStdEdit("", hWnd, hFont, ID_EDT_IN_DEC, ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL, false);

    hBtnDec = CreateModernBtn("DEKRIPSI", hWnd, ID_BTN_DECRYPT, false);
    hLblDecRes = CreateStdLabel("Hasil Dekripsi (Pesan Asli):", hWnd, hFont, false);
    hEdtOutDec = CreateStdEdit("", hWnd, hFont, ID_EDT_OUT_DEC, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, false);
    
    // Langkah Pengerjaan
    hLblCaraEnc = CreateStdLabel("Langkah Pengerjaan (Enkripsi):", hWnd, hFont, false);
    hEdtCaraEnc = CreateStdEdit("", hWnd, hFont, ID_EDT_CARA_ENC, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, false);
    
    hLblCaraDec = CreateStdLabel("Langkah Pengerjaan (Dekripsi):", hWnd, hFont, false);
    hEdtCaraDec = CreateStdEdit("", hWnd, hFont, ID_EDT_CARA_DEC, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, false);
}

int g_layoutBottom = 750;

// ... [Lewatkan definisi hingga UpdateScrollState] ...

// Menyesuaikan margin responsif
void ResizeLayout(int width, int height) {
    if (width < 960) width = 960; 

    int cx = width / 2;
    int cy = (height / 2);

    // Menu Layout
    if (!g_isAppView) {
        MoveWindow(hMenuTitle, 0, cy - 140, width, 50, TRUE);
        MoveWindow(hMenuSub, 0, cy - 80, width, 40, TRUE);
        MoveWindow(hBtnStart, cx - 100, cy, 200, 45, TRUE);
        MoveWindow(hBtnAbout, cx - 100, cy + 60, 200, 45, TRUE);
        MoveWindow(hBtnExit, cx - 100, cy + 120, 200, 45, TRUE);
    }

    // App Screen Layout - Parameter responsibilitas baru
    
    int col1_x = 30; // Jarak kiri
    int gap_x = 20;  // Jarak tengah antar kolom
    int margin_r = 50; // Jarak pinggir kanan ekstra
    
    int wHalf = (width - col1_x - margin_r - gap_x) / 2;
    int col2_x = col1_x + wHalf + gap_x;

    MoveWindow(hAppTitle, col1_x, 15 - g_scrollY, 400, 30, TRUE);
    MoveWindow(hBtnBack, width - margin_r - 160, 15 - g_scrollY, 160, 30, TRUE);

    // Baris 1: P, Q dan Tombol Hitung
    int btnHitungW = 120;
    int colW_PQ = (width - col1_x - gap_x - btnHitungW - gap_x - margin_r) / 2; 
    int yRow1 = 55 - g_scrollY;
    
    MoveWindow(hLblP, col1_x, yRow1, colW_PQ, 20, TRUE);
    MoveWindow(hEdtP, col1_x, yRow1 + 20, colW_PQ, 28, TRUE);
    
    int q_x = col1_x + colW_PQ + gap_x;
    MoveWindow(hLblQ, q_x, yRow1, colW_PQ, 20, TRUE);
    MoveWindow(hEdtQ, q_x, yRow1 + 20, colW_PQ, 28, TRUE);

    int btn_x = q_x + colW_PQ + gap_x;
    MoveWindow(hBtnHitung, btn_x, yRow1, btnHitungW, 48, TRUE); 

    // Label Hasil
    int yHasil = yRow1 + 70;
    MoveWindow(hLblHasil, col1_x, yHasil, 300, 22, TRUE);

    // Baris 2: N & M
    int yRow2 = yHasil + 35;

    MoveWindow(hLblN, col1_x, yRow2, wHalf, 20, TRUE);
    MoveWindow(hEdtN, col1_x, yRow2 + 20, wHalf, 28, TRUE);

    MoveWindow(hLblM, col2_x, yRow2, wHalf, 20, TRUE);
    MoveWindow(hEdtM, col2_x, yRow2 + 20, wHalf, 28, TRUE);

    // Label Struktur Kunci
    int yKunci = yRow2 + 65;
    MoveWindow(hLblSubKunci, col1_x, yKunci, 300, 22, TRUE);

    // Baris 3: E & D
    int yRow3 = yKunci + 35;
    
    // Label E
    MoveWindow(hLblE, col1_x, yRow3, wHalf, 20, TRUE);
    int btnBuatW = 170;
    MoveWindow(hEdtE, col1_x, yRow3 + 20, wHalf - btnBuatW - 10, 28, TRUE);
    MoveWindow(hBtnBuatD, col1_x + wHalf - btnBuatW, yRow3 + 19, btnBuatW, 30, TRUE);

    // Kunci Privat + Checkbox Hide
    MoveWindow(hLblD, col2_x, yRow3, wHalf - 130, 20, TRUE);
    MoveWindow(hChkHideD, col2_x + wHalf - 120, yRow3, 120, 20, TRUE);
    MoveWindow(hEdtD, col2_x, yRow3 + 20, wHalf, 28, TRUE);

    // Seksi Enkripsi & Dekripsi
    int yEnc = yRow3 + 65;
    
    // Alokasi Tinggi Fleksibel (Proportional Fill)
    int yEncRaw = 325; // Absolut posisi yEnc dari layar atas jika tanpa scrollbars (55+70+35+65+35+65=325)
    int remHeight = height - yEncRaw - 30; // 30px jarak dasar aman
    if (remHeight < 400) remHeight = 400;  // Batas minimum agak lebih besar untuk menampung cara pengerjaan
    
    // Fixed gap pada form (Input, Ascii, Cipher, Cara) = ~165px
    int varSpace = remHeight - 165;
    if (varSpace < 360) varSpace = 360;
    
    // Proporsi 110:50:50:150
    int editH_top = (varSpace * 110) / 360;
    int outH = (varSpace * 50) / 360;
    int caraH = (varSpace * 150) / 360;
    
    // Kiri (Enkripsi)
    MoveWindow(hLblEnc, col1_x, yEnc, wHalf, 25, TRUE);
    MoveWindow(hEdtInEnc, col1_x, yEnc + 25, wHalf, editH_top, TRUE);

    int yBtnBase = yEnc + 25 + editH_top + 15;
    MoveWindow(hBtnEnc, col1_x, yBtnBase, 150, 35, TRUE);
    
    MoveWindow(hLblEncAscii, col1_x, yBtnBase + 45, wHalf, 20, TRUE);
    MoveWindow(hEdtOutAscii, col1_x, yBtnBase + 65, wHalf, outH, TRUE);

    int yCipher = yBtnBase + 65 + outH + 10;
    MoveWindow(hLblEncRes, col1_x, yCipher, wHalf, 20, TRUE);
    MoveWindow(hEdtOutEnc, col1_x, yCipher + 20, wHalf, outH, TRUE);

    int yCaraEnc = yCipher + 20 + outH + 10;
    MoveWindow(hLblCaraEnc, col1_x, yCaraEnc, wHalf, 20, TRUE);
    MoveWindow(hEdtCaraEnc, col1_x, yCaraEnc + 20, wHalf, caraH, TRUE);

    // Kanan (Dekripsi)
    MoveWindow(hLblDec, col2_x, yEnc, wHalf, 25, TRUE);
    MoveWindow(hEdtInDec, col2_x, yEnc + 25, wHalf, editH_top, TRUE);

    MoveWindow(hBtnDec, col2_x, yBtnBase, 150, 35, TRUE);
    MoveWindow(hLblDecRes, col2_x, yBtnBase + 45, wHalf, 20, TRUE);
    
    int totalOutH = (yCipher + 20 + outH) - (yBtnBase + 65);
    MoveWindow(hEdtOutDec, col2_x, yBtnBase + 65, wHalf, totalOutH, TRUE);
    
    int yCaraDec = yCaraEnc;
    MoveWindow(hLblCaraDec, col2_x, yCaraDec, wHalf, 20, TRUE);
    MoveWindow(hEdtCaraDec, col2_x, yCaraDec + 20, wHalf, caraH, TRUE);
    
    g_layoutBottom = yCaraEnc + 20 + caraH + g_scrollY; // Menyimpan hitungan sejati layar (bebas offset)
}

void UpdateScrollState(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    si.nMin = 0;
    
    if (g_isAppView) {
        si.nMax = g_layoutBottom + 30; // Jarak total logical element saat aktif App
        si.nPos = 0;
        si.fMask |= SIF_POS;
    } else {
        si.nMax = 0; // Matikan scroll pada Menu layaknya layar statis
        si.nPos = 0;
        si.fMask |= SIF_POS;
        g_scrollY = 0;
    }
    
    si.nPage = rect.bottom;
    SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
    GetScrollInfo(hWnd, SB_VERT, &si);
    g_scrollY = si.nPos;
}

void SwitchView(HWND hWnd, bool showMenu) {
    g_isAppView = !showMenu;
    int menuShowState = showMenu ? SW_SHOW : SW_HIDE;
    int appShowState = showMenu ? SW_HIDE : SW_SHOW;
    
    for (HWND h : menuControls) ShowWindow(h, menuShowState);
    for (HWND h : appControls) ShowWindow(h, appShowState);
    
    RECT rect;
    GetClientRect(hWnd, &rect);
    ResizeLayout(rect.right, rect.bottom); // Hitung logical span pertama kali
    UpdateScrollState(hWnd); // Aktifkan/matikan Scroll
    ResizeLayout(rect.right, rect.bottom); // Pass kedua apply offset true
    InvalidateRect(hWnd, NULL, TRUE);
}

void DrawModernButton(LPDRAWITEMSTRUCT pdis) {
// [Lewati, sudah ada di source, kita tidak sentuh isinya agar Replace tetap tepat]
// Akan tetapi replace tool tidak support wildcards melainkan persis substring target.
// Oleh karena itu saya akan target tepat dari bagian fungsi ResizeLayout.
    HDC hdc = pdis->hDC;
    RECT rect = pdis->rcItem;
    bool isPressed = pdis->itemState & ODS_SELECTED;
    BtnState* state = (BtnState*)GetPropA(pdis->hwndItem, "BTN_STATE");
    float alpha = state ? state->hoverAlpha : 0.0f;

    COLORREF baseColor = BlendColor(BTN_BG, BTN_HOVER, alpha);
    COLORREF drawBorder = BORDER_COLOR;
    COLORREF drawText = TEXT_COLOR;

    COLORREF bgColor = isPressed ? BTN_DOWN : baseColor;

    // Refresh background button area dengan BG_COLOR agar lengkungan terlihat mulus
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(hdc, &pdis->rcItem, bgBrush);
    DeleteObject(bgBrush);

    if (isPressed) {
        rect.top += 1;
        rect.left += 1;
    }

    HBRUSH brush = CreateSolidBrush(bgColor);
    
    // Matikan penggambaran garis border karena rasterisasi native GDI padanya terlalu kasar
    HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    
    // Gunakan Radius ideal bernilai 8px untuk menjaga kekokohan layout modern UI 
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 8, 8);
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);

    char btnText[64];
    GetWindowTextA(pdis->hwndItem, btnText, sizeof(btnText));
    
    SetTextColor(hdc, drawText);
    SetBkMode(hdc, TRANSPARENT);
    
    if (pdis->CtlID == ID_BTN_BUAT_D) {
        HFONT hSmallFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
        SelectObject(hdc, hSmallFont);
        DrawTextA(hdc, btnText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(hSmallFont);
    } else {
        SelectObject(hdc, hFont);
        DrawTextA(hdc, btnText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
}

