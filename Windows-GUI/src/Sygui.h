#ifdef _WIN32
#ifndef JUGUI_H
#define JUGUI_H

// 强制使用宽字符版本的Windows API
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <string>
#include <functional>
#include <vector>
#include <commdlg.h>
#include <tchar.h>
namespace SyLib
{
    // 封装Windows GUI的核心类
    class WinGUI
    {
    private:
        // 窗口核心属性
        HWND hWnd;
        std::wstring className;
        std::wstring windowTitle;
        int width;
        int height;
        HINSTANCE hInstance;

        // 窗口过程回调
        std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> wndProcCallback;

        // 静态窗口过程（系统回调入口）
        static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            WinGUI *pThis = nullptr;
            if (uMsg == WM_NCCREATE)
            {
                // 使用CREATESTRUCTW明确宽字符版本
                CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
                pThis = static_cast<WinGUI *>(pCreate->lpCreateParams);

                // 兼容32/64位存储窗口对象指针
#ifdef _WIN64
                SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
#else
                SetWindowLongW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG>(pThis));
#endif
            }
            else
            {
                // 兼容32/64位读取窗口对象指针
#ifdef _WIN64
                pThis = reinterpret_cast<WinGUI *>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
#else
                pThis = reinterpret_cast<WinGUI *>(GetWindowLongW(hWnd, GWLP_USERDATA));
#endif
            }

            if (pThis && pThis->wndProcCallback)
            {
                return pThis->wndProcCallback(hWnd, uMsg, wParam, lParam);
            }
            // 使用全局命名空间的宽字符版本
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        // 注册窗口类（内部调用）
        bool RegisterWindowClass()
        {
            WNDCLASSEXW wc = {0};
            wc.cbSize = sizeof(WNDCLASSEXW);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.lpfnWndProc = StaticWndProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;
            wc.hInstance = hInstance;
            wc.hIcon = ::LoadIconW(nullptr, IDI_APPLICATION);
            wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
            wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            wc.lpszMenuName = nullptr;
            wc.lpszClassName = className.c_str();
            wc.hIconSm = ::LoadIconW(nullptr, IDI_APPLICATION);

            if (!::RegisterClassExW(&wc))
            {
                ShowErrorMsg(L"窗口类注册失败", ::GetLastError());
                return false;
            }
            return true;
        }

    public:
        // 控件ID枚举（方便管理）
        enum ControlID
        {
            ID_BUTTON_BASE = 1000,
            ID_EDIT_BASE = 2000,
            ID_STATIC_BASE = 3000,
            ID_LISTBOX_BASE = 4000,
            ID_MENU_BASE = 5000
        };

        // 构造函数
        WinGUI(const std::wstring &clsName = L"WinGUI_Class",
               const std::wstring &title = L"WinGUI窗口",
               int w = 800, int h = 600)
            : className(clsName), windowTitle(title), width(w), height(h),
              hWnd(nullptr), hInstance(nullptr)
        {
            // 默认窗口过程（基础消息处理）
            wndProcCallback = [this](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
            {
                switch (uMsg)
                {
                case WM_CLOSE:
                    ::DestroyWindow(hWnd);
                    return 0;
                case WM_DESTROY:
                    ::PostQuitMessage(0);
                    return 0;
                case WM_COMMAND:
                    OnCommand(LOWORD(wParam), HIWORD(wParam), lParam);
                    return 0;
                default:
                    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
                }
            };
        }

        // 析构函数
        ~WinGUI()
        {
            if (hWnd)
            {
                ::DestroyWindow(hWnd);
            }
            if (hInstance)
            {
                ::UnregisterClassW(className.c_str(), hInstance);
            }
        }

        // ===================== 核心窗口操作 =====================
        // 创建窗口
        bool Create(HINSTANCE hInst, int nCmdShow)
        {
            hInstance = hInst;
            if (!RegisterWindowClass())
            {
                return false;
            }

            hWnd = ::CreateWindowExW(
                0,                            // 扩展样式
                className.c_str(),            // 窗口类名（宽字符）
                windowTitle.c_str(),          // 窗口标题（宽字符）
                WS_OVERLAPPEDWINDOW,          // 窗口样式
                CW_USEDEFAULT, CW_USEDEFAULT, // 窗口位置
                width, height,                // 窗口大小
                nullptr,                      // 父窗口
                nullptr,                      // 菜单
                hInstance,                    // 实例句柄
                this                          // 传递给窗口过程的参数
            );

            if (!hWnd)
            {
                ShowErrorMsg(L"窗口创建失败", ::GetLastError());
                return false;
            }

            ::ShowWindow(hWnd, nCmdShow);
            ::UpdateWindow(hWnd);
            return true;
        }

        // 运行消息循环
        int RunMessageLoop()
        {
            MSG msg = {0};
            // 使用GetMessageW确保宽字符消息处理
            while (::GetMessageW(&msg, nullptr, 0, 0))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageW(&msg);
            }
            return static_cast<int>(msg.wParam);
        }

        // 设置自定义窗口过程
        void SetCustomWndProc(const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> &callback)
        {
            wndProcCallback = callback;
        }

        // ===================== 基础控件创建 =====================
        // 创建按钮
        HWND CreateButtonCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
        {
            if (!hWnd)
                return nullptr;
            return ::CreateWindowW(
                L"BUTTON",    // 宽字符类名
                text.c_str(), // 宽字符文本
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                x, y, w, h,
                hWnd,
                reinterpret_cast<HMENU>(id),
                hInstance,
                nullptr);
        }

        // 创建静态文本
        HWND CreateStaticTextCtrl(const std::wstring &text, int x, int y, int w, int h, int id)
        {
            if (!hWnd)
                return nullptr;
            return ::CreateWindowW(
                L"STATIC",
                text.c_str(),
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                x, y, w, h,
                hWnd,
                reinterpret_cast<HMENU>(id),
                hInstance,
                nullptr);
        }

        // 创建编辑框（支持多行/密码/只读）
        HWND CreateEditBoxCtrl(int x, int y, int w, int h, int id, bool isMultiLine = false,
                               bool isPassword = false, bool isReadOnly = false)
        {
            if (!hWnd)
                return nullptr;

            DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
            if (isMultiLine)
                style |= ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL;
            if (isPassword)
                style |= ES_PASSWORD;
            if (isReadOnly)
                style |= ES_READONLY;

            return ::CreateWindowW(
                L"EDIT",
                L"", // 空宽字符字符串
                style,
                x, y, w, h,
                hWnd,
                reinterpret_cast<HMENU>(id),
                hInstance,
                nullptr);
        }

        // 创建列表框
        HWND CreateListBoxCtrl(int x, int y, int w, int h, int id, bool isSort = true)
        {
            if (!hWnd)
                return nullptr;

            DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL;
            if (isSort)
                style |= LBS_SORT;

            return ::CreateWindowW(
                L"LISTBOX",
                L"",
                style,
                x, y, w, h,
                hWnd,
                reinterpret_cast<HMENU>(id),
                hInstance,
                nullptr);
        }

        // ===================== 控件操作 =====================
        // 设置控件文本
        bool SetCtrlText(HWND hCtrl, const std::wstring &text)
        {
            if (!hCtrl)
                return false;
            return ::SetWindowTextW(hCtrl, text.c_str()) != 0;
        }

        // 获取控件文本
        std::wstring GetCtrlText(HWND hCtrl)
        {
            if (!hCtrl)
                return L"";
            int len = ::GetWindowTextLengthW(hCtrl) + 1;
            std::vector<wchar_t> buf(len);
            ::GetWindowTextW(hCtrl, buf.data(), len);
            return std::wstring(buf.data());
        }

        // 列表框添加项
        int ListBoxAddItemCtrl(HWND hListBox, const std::wstring &text)
        {
            if (!hListBox)
                return -1;
            return ::SendMessageW(hListBox, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
        }

        // 列表框删除选中项
        void ListBoxDeleteSelectedCtrl(HWND hListBox)
        {
            if (!hListBox)
                return;
            int idx = ::SendMessageW(hListBox, LB_GETCURSEL, 0, 0);
            if (idx != LB_ERR)
            {
                ::SendMessageW(hListBox, LB_DELETESTRING, idx, 0);
            }
        }

        // ===================== 菜单操作 =====================
        // 创建主菜单
        HMENU CreateMainMenuCtrl(const std::vector<std::pair<std::wstring, std::vector<std::pair<std::wstring, int>>>> &menuItems)
        {
            if (!hWnd)
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
            ::SetMenu(hWnd, hMenu);
            ::DrawMenuBar(hWnd);
            return hMenu;
        }

        // ===================== 对话框/提示框 =====================
        // 显示错误提示框（带错误码）
        void ShowErrorMsg(const std::wstring &msg, DWORD errorCode = 0)
        {
            std::wstring fullMsg = msg;
            if (errorCode != 0)
            {
                wchar_t errBuf[256] = {0};
                ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, errBuf, 256, nullptr);
                fullMsg += L"\n错误码: " + std::to_wstring(errorCode) + L"\n" + errBuf;
            }
            ::MessageBoxW(hWnd, fullMsg.c_str(), L"错误", MB_ICONERROR);
        }

        // 显示信息提示框
        int ShowInfoMsg(const std::wstring &msg, const std::wstring &title = L"提示")
        {
            return ::MessageBoxW(hWnd, msg.c_str(), title.c_str(), MB_ICONINFORMATION);
        }

        // 显示确认对话框（是/否）
        bool ShowConfirmMsg(const std::wstring &msg, const std::wstring &title = L"确认")
        {
            return ::MessageBoxW(hWnd, msg.c_str(), title.c_str(), MB_YESNO | MB_ICONQUESTION) == IDYES;
        }

        // 打开文件选择对话框
        std::wstring OpenFileDialogCtrl(const std::wstring &filter = L"所有文件 (*.*)\0*.*\0文本文件 (*.txt)\0*.txt\0")
        {
            OPENFILENAMEW ofn = {0};
            wchar_t filePath[MAX_PATH] = {0};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hWnd;
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

        // 保存文件选择对话框
        std::wstring SaveFileDialogCtrl(const std::wstring &filter = L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0")
        {
            OPENFILENAMEW ofn = {0};
            wchar_t filePath[MAX_PATH] = {0};
            ofn.lStructSize = sizeof(OPENFILENAMEW);
            ofn.hwndOwner = hWnd;
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

        // ===================== 窗口样式/状态控制 =====================
        // 设置窗口标题
        void SetWndTitle(const std::wstring &title)
        {
            windowTitle = title;
            if (hWnd)
            {
                ::SetWindowTextW(hWnd, title.c_str());
            }
        }

        // 设置窗口大小和位置（彻底重命名避免冲突）
        void MoveResizeWindow(int x, int y, int w, int h, bool repaint = true)
        {
            if (!hWnd)
                return;
            ::SetWindowPos(hWnd, HWND_TOP, x, y, w, h,
                           repaint ? SWP_NOZORDER : SWP_NOZORDER | SWP_NOREDRAW);
        }

        // 窗口居中
        void CenterWnd()
        {
            if (!hWnd)
                return;
            RECT rect = {0};
            ::GetWindowRect(hWnd, &rect);
            int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);
            int x = (screenWidth - (rect.right - rect.left)) / 2;
            int y = (screenHeight - (rect.bottom - rect.top)) / 2;
            ::SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }

        // 显示/隐藏窗口
        void ShowHideWnd(bool show)
        {
            if (!hWnd)
                return;
            ::ShowWindow(hWnd, show ? SW_SHOW : SW_HIDE);
        }

        // 启用/禁用窗口
        void EnableDisableWnd(bool enable)
        {
            if (!hWnd)
                return;
            ::EnableWindow(hWnd, enable);
        }

        // ===================== 绘图功能 =====================
        // 获取窗口DC
        HDC GetWindowDCHandle()
        {
            return hWnd ? ::GetWindowDC(hWnd) : nullptr;
        }

        // 释放DC
        void ReleaseWindowDCHandle(HDC hdc)
        {
            if (hWnd && hdc)
            {
                ::ReleaseDC(hWnd, hdc);
            }
        }

        // 绘制文本
        void DrawTextToWnd(const std::wstring &text, int x, int y, COLORREF color = RGB(0, 0, 0))
        {
            if (!hWnd)
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

        // 绘制矩形
        void DrawRectToWnd(int x, int y, int w, int h, COLORREF color = RGB(0, 0, 0), bool fill = false)
        {
            if (!hWnd)
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

        // ===================== 事件处理（可重写） =====================
        // 命令事件处理（按钮/菜单点击）
        void OnCommand(int ctrlId, int notifyCode, LPARAM lParam)
        {
            return;
        }

        // ===================== 辅助方法 =====================
        // 获取窗口句柄
        HWND GetWndHandle() const { return hWnd; }

        // 获取实例句柄
        HINSTANCE GetInstHandle() const { return hInstance; }
    };
}
#endif // JUGUI_H
#else
#error "This OS isn't Windows!"
#endif