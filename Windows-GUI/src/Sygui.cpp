#include "Sygui.h"
#include <commdlg.h>
#include <tchar.h>

namespace SyLib
{
    WinGUI::WinGUI(const std::wstring &clsName, const std::wstring &title, int w, int h)
        : m_className(clsName), m_windowTitle(title), m_width(w), m_height(h), m_hWnd(nullptr), m_hInstance(nullptr)
    {
        signal_wndProc.slots.clear();
        signal_command.slots.clear();
        signal_wndClose.slots.clear();
        signal_wndDestroy.slots.clear();
        signal_wndPaint.slots.clear();
        signal_timer.slots.clear();
        signal_mouseMove.slots.clear();
        signal_lButtonDown.slots.clear();
        signal_rButtonDown.slots.clear();
        signal_lButtonUp.slots.clear();
        signal_rButtonUp.slots.clear();
        signal_keyDown.slots.clear();
        signal_wndSize.slots.clear();
        signal_dropFiles.slots.clear();
        signal_mouseWheel.slots.clear();
        signal_captureChanged.slots.clear();

        signal_wndProc.blocked = false;
        signal_command.blocked = false;
        signal_wndClose.blocked = false;
        signal_wndDestroy.blocked = false;
        signal_wndPaint.blocked = false;
        signal_timer.blocked = false;
        signal_mouseMove.blocked = false;
        signal_lButtonDown.blocked = false;
        signal_rButtonDown.blocked = false;
        signal_lButtonUp.blocked = false;
        signal_rButtonUp.blocked = false;
        signal_keyDown.blocked = false;
        signal_wndSize.blocked = false;
        signal_dropFiles.blocked = false;
        signal_mouseWheel.blocked = false;
        signal_captureChanged.blocked = false;
    }

    WinGUI::~WinGUI()
    {
        if (m_hWnd)
        {
            ::DestroyWindow(m_hWnd);
        }
        if (m_hInstance)
        {
            ::UnregisterClassW(m_className.c_str(), m_hInstance);
        }
    }

    bool WinGUI::RegisterWindowClass()
    {
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = m_hInstance;
        wc.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
        wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = m_className.c_str();
        wc.hIconSm = ::LoadIconW(nullptr, IDI_APPLICATION);

        if (!::RegisterClassExW(&wc))
        {
            ShowErrorMsg(L"Failed to register window class.", ::GetLastError());
            return false;
        }
        return true;
    }

    UINT_PTR WinGUI::StartTimer(UINT_PTR timerId, UINT elapseMs)
    {
        if (!m_hWnd)
            return 0;
        return ::SetTimer(m_hWnd, timerId, elapseMs, nullptr);
    }

    void WinGUI::StopTimer(UINT_PTR timerId)
    {
        if (m_hWnd)
        {
            ::KillTimer(m_hWnd, timerId);
        }
    }

    void WinGUI::EnableFileDrop(bool enable)
    {
        if (!m_hWnd)
            return;
        LONG_PTR exStyle = ::GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
        if (enable)
            exStyle |= WS_EX_ACCEPTFILES;
        else
            exStyle &= ~WS_EX_ACCEPTFILES;
        ::SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, exStyle);
    }

    LRESULT CALLBACK WinGUI::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        WinGUI *pThis = nullptr;
        if (uMsg == WM_NCCREATE)
        {
            CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
            pThis = static_cast<WinGUI *>(pCreate->lpCreateParams);
#ifdef _WIN64
            SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
#else
            SetWindowLongW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pThis));
#endif
        }
        else
        {
#ifdef _WIN64
            pThis = reinterpret_cast<WinGUI *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
#else
            pThis = reinterpret_cast<WinGUI *>(GetWindowLongW(hWnd, GWLP_USERDATA));
#endif
        }
        if (!pThis)
        {
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        switch (uMsg)
        {
        case WM_CLOSE:
        {
            if (!pThis->signal_wndClose.blocked)
            {
                for (auto &slotWrap : pThis->signal_wndClose.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd);
            }
            ::DestroyWindow(hWnd);
            return 0;
        }
        case WM_DESTROY:
        {
            if (!pThis->signal_wndDestroy.blocked)
            {
                for (auto &slotWrap : pThis->signal_wndDestroy.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd);
            }
            ::PostQuitMessage(0);
            return 0;
        }
        case WM_COMMAND:
        {
            int ctrlId = LOWORD(wParam);
            int notifyCode = HIWORD(wParam);
            HWND hCtrl = reinterpret_cast<HWND>(lParam);
            if (!pThis->signal_command.blocked)
            {
                for (auto &slotWrap : pThis->signal_command.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(ctrlId, notifyCode, hCtrl);
            }
            return 0;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hWnd, &ps);
            if (!pThis->signal_wndPaint.blocked)
            {
                for (auto &slotWrap : pThis->signal_wndPaint.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, hdc);
            }
            ::EndPaint(hWnd, &ps);
            return 0;
        }
        case WM_TIMER:
        {
            UINT_PTR tid = reinterpret_cast<UINT_PTR>(wParam);
            if (!pThis->signal_timer.blocked)
            {
                for (auto &slotWrap : pThis->signal_timer.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(tid);
            }
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam);
            if (!pThis->signal_mouseMove.blocked)
            {
                for (auto &slotWrap : pThis->signal_mouseMove.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, x, y, keyFlags);
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam);
            if (!pThis->signal_lButtonDown.blocked)
            {
                for (auto &slotWrap : pThis->signal_lButtonDown.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, x, y, keyFlags);
            }
            return 0;
        }
        case WM_RBUTTONDOWN:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam);
            if (!pThis->signal_rButtonDown.blocked)
            {
                for (auto &slotWrap : pThis->signal_rButtonDown.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, x, y, keyFlags);
            }
            return 0;
        }
        case WM_LBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam);
            if (!pThis->signal_lButtonUp.blocked)
            {
                for (auto &slotWrap : pThis->signal_lButtonUp.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, x, y, keyFlags);
            }
            return 0;
        }
        case WM_RBUTTONUP:
        {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam);
            if (!pThis->signal_rButtonUp.blocked)
            {
                for (auto &slotWrap : pThis->signal_rButtonUp.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, x, y, keyFlags);
            }
            return 0;
        }
        case WM_KEYDOWN:
        {
            UINT vkCode = static_cast<UINT>(wParam);
            UINT repeat = static_cast<UINT>(HIWORD(lParam) & 0xFFFF);
            UINT flags = static_cast<UINT>((lParam >> 16) & 0xFFFF);
            if (!pThis->signal_keyDown.blocked)
            {
                for (auto &slotWrap : pThis->signal_keyDown.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, vkCode, repeat, flags);
            }
            return 0;
        }
        case WM_SIZE:
        {
            UINT sizeType = static_cast<UINT>(wParam);
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (!pThis->signal_wndSize.blocked)
            {
                for (auto &slotWrap : pThis->signal_wndSize.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, sizeType, w, h);
            }
            return 0;
        }
        case WM_DROPFILES:
        {
            HDROP hDrop = reinterpret_cast<HDROP>(wParam);
            std::vector<std::wstring> fileList;
            UINT fileCount = ::DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
            for (UINT i = 0; i < fileCount; ++i)
            {
                UINT bufLen = ::DragQueryFileW(hDrop, i, nullptr, 0);
                std::vector<wchar_t> buf(bufLen + 1);
                ::DragQueryFileW(hDrop, i, buf.data(), static_cast<UINT>(buf.size()));
                fileList.emplace_back(buf.data());
            }
            if (!pThis->signal_dropFiles.blocked)
            {
                for (auto &slotWrap : pThis->signal_dropFiles.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, fileList);
            }
            ::DragFinish(hDrop);
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            // MinGW‑w64规避损坏GET_WHEEL_DELTA_WPARAM宏
            int delta = static_cast<int>(static_cast<short>(HIWORD(wParam)));
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            UINT keyFlags = static_cast<UINT>(wParam & 0xFFFF);
            if (!pThis->signal_mouseWheel.blocked)
            {
                for (auto &slotWrap : pThis->signal_mouseWheel.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, delta, x, y, keyFlags);
            }
            return 0;
        }
        case WM_CAPTURECHANGED:
        {
            HWND hwndLost = reinterpret_cast<HWND>(lParam);
            if (!pThis->signal_captureChanged.blocked)
            {
                for (auto &slotWrap : pThis->signal_captureChanged.slots)
                    if (slotWrap.fn)
                        slotWrap.fn(hWnd, hwndLost);
            }
            return 0;
        }
        default:
            break;
        }

        // signal_wndProc（支持LRESULT返回值，修复void赋值编译错误）
        LRESULT procRet = 0;
        bool hasProc = false;
        if (!pThis->signal_wndProc.blocked)
        {
            for (auto &slotWrap : pThis->signal_wndProc.slots)
            {
                if (slotWrap.fn)
                {
                    LRESULT r = slotWrap.fn(hWnd, uMsg, wParam, lParam);
                    if (r != 0)
                    {
                        procRet = r;
                        hasProc = true;
                    }
                }
            }
        }
        if (hasProc)
        {
            return procRet;
        }

        return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    bool WinGUI::Create(HINSTANCE hInst, int nCmdShow)
    {
        m_hInstance = hInst;
        if (!RegisterWindowClass())
            return false;

        m_hWnd = ::CreateWindowExW(
            0,
            m_className.c_str(),
            m_windowTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            m_width, m_height,
            nullptr,
            nullptr,
            m_hInstance,
            this);
        if (!m_hWnd)
        {
            ShowErrorMsg(L"Failed to create window.", ::GetLastError());
            return false;
        }
        ::ShowWindow(m_hWnd, nCmdShow);
        ::UpdateWindow(m_hWnd);
        return true;
    }

    int WinGUI::RunMessageLoop()
    {
        MSG msg = {0};
        while (::GetMessageW(&msg, nullptr, 0, 0))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

    HWND WinGUI::CreateButtonCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
    {
        if (!m_hWnd)
            return nullptr;
        return ::CreateWindowW(L"BUTTON", text.c_str(),
                               WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateStaticTextCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
    {
        if (!m_hWnd)
            return nullptr;
        return ::CreateWindowW(L"STATIC", text.c_str(),
                               WS_CHILD | WS_VISIBLE | SS_LEFT,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateEditBoxCtrl(int x, int y, int w, int h, int id, bool isMultiLine, bool isPassword, bool isReadOnly)
    {
        if (!m_hWnd)
            return nullptr;
        DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
        if (isMultiLine)
            style |= ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL;
        if (isPassword)
            style |= ES_PASSWORD;
        if (isReadOnly)
            style |= ES_READONLY;
        return ::CreateWindowW(L"EDIT", L"", style,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateListBoxCtrl(int x, int y, int w, int h, int id, bool isSort)
    {
        if (!m_hWnd)
            return nullptr;
        DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL;
        if (isSort)
            style |= LBS_SORT;
        return ::CreateWindowW(L"LISTBOX", L"", style,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateCheckBoxCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
    {
        if (!m_hWnd)
            return nullptr;
        return ::CreateWindowW(L"BUTTON", text.c_str(),
                               WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateRadioButtonCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
    {
        if (!m_hWnd)
            return nullptr;
        return ::CreateWindowW(L"BUTTON", text.c_str(),
                               WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    HWND WinGUI::CreateComboBoxCtrl(int x, int y, int w, int h, int id, bool isDropList)
    {
        if (!m_hWnd)
            return nullptr;
        DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER;
        style |= isDropList ? CBS_DROPDOWNLIST : CBS_DROPDOWN;
        return ::CreateWindowW(L"COMBOBOX", L"", style,
                               x, y, w, h,
                               m_hWnd, reinterpret_cast<HMENU>(id), m_hInstance, nullptr);
    }

    bool WinGUI::SetCheckBoxCheck(HWND hCtrl, bool checked)
    {
        if (!hCtrl)
            return false;
        ::SendMessageW(hCtrl, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        return true;
    }

    bool WinGUI::GetCheckBoxCheck(HWND hCtrl)
    {
        if (!hCtrl)
            return false;
        LRESULT r = ::SendMessageW(hCtrl, BM_GETCHECK, 0, 0);
        return r == BST_CHECKED;
    }

    int WinGUI::ComboBoxAddItem(HWND hCombo, const std::wstring &text)
    {
        if (!hCombo)
            return -1;
        return static_cast<int>(::SendMessageW(hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str())));
    }

    void WinGUI::ComboBoxDeleteSelected(HWND hCombo)
    {
        if (!hCombo)
            return;
        int idx = static_cast<int>(::SendMessageW(hCombo, CB_GETCURSEL, 0, 0));
        if (idx != CB_ERR)
            ::SendMessageW(hCombo, CB_DELETESTRING, idx, 0);
    }

    int WinGUI::ComboBoxGetSelectedIndex(HWND hCombo)
    {
        if (!hCombo)
            return CB_ERR;
        return static_cast<int>(::SendMessageW(hCombo, CB_GETCURSEL, 0, 0));
    }

    bool WinGUI::ComboBoxSetSelectedIndex(HWND hCombo, int idx)
    {
        if (!hCombo)
            return false;
        LRESULT ret = ::SendMessageW(hCombo, CB_SETCURSEL, idx, 0);
        return ret != CB_ERR;
    }

    bool WinGUI::SetCtrlText(HWND hCtrl, const std::wstring &text)
    {
        if (!hCtrl)
            return false;
        return ::SetWindowTextW(hCtrl, text.c_str()) != 0;
    }

    std::wstring WinGUI::GetCtrlText(HWND hCtrl)
    {
        if (!hCtrl)
            return L"";
        int len = ::GetWindowTextLengthW(hCtrl) + 1;
        std::vector<wchar_t> buf(len);
        ::GetWindowTextW(hCtrl, buf.data(), len);
        return std::wstring(buf.data());
    }

    int WinGUI::ListBoxAddItemCtrl(HWND hListBox, const std::wstring &text)
    {
        if (!hListBox)
            return -1;
        return static_cast<int>(::SendMessageW(hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str())));
    }

    void WinGUI::ListBoxDeleteSelectedCtrl(HWND hListBox)
    {
        if (!hListBox)
            return;
        int idx = static_cast<int>(::SendMessageW(hListBox, LB_GETCURSEL, 0, 0));
        if (idx != LB_ERR)
        {
            ::SendMessageW(hListBox, LB_DELETESTRING, idx, 0);
        }
    }

    HMENU WinGUI::CreateMainMenuCtrl(const std::vector<std::pair<std::wstring, std::vector<std::pair<std::wstring, int>>>> &menuItems)
    {
        if (!m_hWnd)
            return nullptr;
        HMENU hMenu = ::CreateMenu();
        for (const auto &menu : menuItems)
        {
            HMENU hSubMenu = ::CreatePopupMenu();
            for (const auto &item : menu.second)
            {
                ::AppendMenuW(hSubMenu, MF_STRING, item.second, item.first.c_str());
            }
            ::AppendMenuW(hMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(hSubMenu), menu.first.c_str());
        }
        ::SetMenu(m_hWnd, hMenu);
        ::DrawMenuBar(m_hWnd);
        return hMenu;
    }

    void WinGUI::ShowErrorMsg(const std::wstring &msg, DWORD errorCode)
    {
        std::wstring fullMsg = msg;
        if (errorCode != 0)
        {
            wchar_t errBuf[256] = {0};
            ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, errBuf, 256, nullptr);
            fullMsg += L"\nError Code: " + std::to_wstring(errorCode) + L"\n" + errBuf;
        }
        ::MessageBoxW(m_hWnd, fullMsg.c_str(), L"Error", MB_ICONERROR);
    }

    int WinGUI::ShowInfoMsg(const std::wstring &msg, const std::wstring &title)
    {
        return ::MessageBoxW(m_hWnd, msg.c_str(), title.c_str(), MB_ICONINFORMATION);
    }

    bool WinGUI::ShowConfirmMsg(const std::wstring &msg, const std::wstring &title)
    {
        return ::MessageBoxW(m_hWnd, msg.c_str(), title.c_str(), MB_YESNO | MB_ICONQUESTION) == IDYES;
    }

    std::wstring WinGUI::OpenFileDialogCtrl(const std::wstring &filter)
    {
        OPENFILENAMEW ofn = {0};
        wchar_t filePath[MAX_PATH] = {0};
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = filter.c_str();
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"*";
        if (::GetOpenFileNameW(&ofn))
        {
            return std::wstring(filePath);
        }
        return L"";
    }

    std::wstring WinGUI::SaveFileDialogCtrl(const std::wstring &filter)
    {
        OPENFILENAMEW ofn = {0};
        wchar_t filePath[MAX_PATH] = {0};
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = m_hWnd;
        ofn.lpstrFilter = filter.c_str();
        ofn.lpstrFile = filePath;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        ofn.lpstrDefExt = L"txt";
        if (::GetSaveFileNameW(&ofn))
        {
            return std::wstring(filePath);
        }
        return L"";
    }

    void WinGUI::SetWndTitle(const std::wstring &title)
    {
        m_windowTitle = title;
        if (m_hWnd)
        {
            ::SetWindowTextW(m_hWnd, title.c_str());
        }
    }

    void WinGUI::MoveResizeWindow(int x, int y, int w, int h, bool repaint)
    {
        if (!m_hWnd)
            return;
        UINT flag = SWP_NOZORDER;
        if (!repaint)
            flag |= SWP_NOREDRAW;
        ::SetWindowPos(m_hWnd, HWND_TOP, x, y, w, h, flag);
    }

    void WinGUI::CenterWnd()
    {
        if (!m_hWnd)
            return;
        RECT rect = {0};
        ::GetWindowRect(m_hWnd, &rect);
        int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
        int x = (screenWidth - (rect.right - rect.left)) / 2;
        int y = (screenHeight - (rect.bottom - rect.top)) / 2;
        ::SetWindowPos(m_hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }

    void WinGUI::ShowHideWnd(bool show)
    {
        if (!m_hWnd)
            return;
        ::ShowWindow(m_hWnd, show ? SW_SHOW : SW_HIDE);
    }

    void WinGUI::EnableDisableWnd(bool enable)
    {
        if (!m_hWnd)
            return;
        ::EnableWindow(m_hWnd, enable);
    }

    HDC WinGUI::GetWindowDCHandle()
    {
        return m_hWnd ? ::GetWindowDC(m_hWnd) : nullptr;
    }

    void WinGUI::ReleaseWindowDCHandle(HDC hdc)
    {
        if (m_hWnd && hdc)
        {
            ::ReleaseDC(m_hWnd, hdc);
        }
    }

    void WinGUI::DrawTextToWnd(const std::wstring &text, int x, int y, COLORREF color)
    {
        if (!m_hWnd)
            return;
        HDC hdc = GetWindowDCHandle();
        if (!hdc)
            return;
        ::SetTextColor(hdc, color);
        ::SetBkMode(hdc, TRANSPARENT);
        RECT textRect = {x, y, x + static_cast<int>(text.length()) * 10, y + 20};
        ::DrawTextW(hdc, text.c_str(), -1, &textRect, DT_LEFT);
        ReleaseWindowDCHandle(hdc);
    }

    void WinGUI::DrawRectToWnd(int x, int y, int w, int h, COLORREF color, bool fill)
    {
        if (!m_hWnd)
            return;
        HDC hdc = GetWindowDCHandle();
        if (!hdc)
            return;
        HPEN hPen = ::CreatePen(PS_SOLID, 1, color);
        HBRUSH hBrush = fill ? ::CreateSolidBrush(color) : reinterpret_cast<HBRUSH>(::GetStockObject(NULL_BRUSH));

        HPEN hOldPen = reinterpret_cast<HPEN>(::SelectObject(hdc, hPen));
        HBRUSH hOldBrush = reinterpret_cast<HBRUSH>(::SelectObject(hdc, hBrush));

        ::Rectangle(hdc, x, y, x + w, y + h);

        ::SelectObject(hdc, hOldPen);
        ::SelectObject(hdc, hOldBrush);
        ::DeleteObject(hPen);
        if (fill)
            ::DeleteObject(hBrush);

        ReleaseWindowDCHandle(hdc);
    }
}
