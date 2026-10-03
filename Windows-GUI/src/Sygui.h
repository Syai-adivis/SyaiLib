#ifdef _WIN32
#ifndef SYGUI_H
#define SYGUI_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <string>
#include <functional>
#include <vector>
#include <cstdint>
#include <utility>
#include <type_traits>

namespace SyLib
{
    using SlotHandle = uint64_t;
    inline SlotHandle NextSlotHandle()
    {
        static SlotHandle g_id = 1;
        return g_id++;
    }

    // ===================== RAII GDI资源：画笔 GdiPen =====================
    class GdiPen
    {
    private:
        HPEN m_hPen{nullptr};

    public:
        GdiPen() = default;
        explicit GdiPen(HPEN hPen) : m_hPen(hPen) {}

        GdiPen(const GdiPen &) = delete;
        GdiPen &operator=(const GdiPen &) = delete;

        GdiPen(GdiPen &&other) noexcept
        {
            m_hPen = other.m_hPen;
            other.m_hPen = nullptr;
        }
        GdiPen &operator=(GdiPen &&other) noexcept
        {
            Reset();
            m_hPen = other.m_hPen;
            other.m_hPen = nullptr;
            return *this;
        }

        ~GdiPen() { Reset(); }

        void Reset()
        {
            if (m_hPen)
            {
                ::DeleteObject(m_hPen);
                m_hPen = nullptr;
            }
        }

        HPEN Get() const { return m_hPen; }
        operator HPEN() const { return m_hPen; }

        static GdiPen Solid(int width, COLORREF color)
        {
            return GdiPen(::CreatePen(PS_SOLID, width, color));
        }
    };

    // ===================== RAII GDI资源：画刷 GdiBrush =====================
    class GdiBrush
    {
    private:
        HBRUSH m_hBrush{nullptr};

    public:
        GdiBrush() = default;
        explicit GdiBrush(HBRUSH hBrush) : m_hBrush(hBrush) {}

        GdiBrush(const GdiBrush &) = delete;
        GdiBrush &operator=(const GdiBrush &) = delete;

        GdiBrush(GdiBrush &&other) noexcept
        {
            m_hBrush = other.m_hBrush;
            other.m_hBrush = nullptr;
        }
        GdiBrush &operator=(GdiBrush &&other) noexcept
        {
            Reset();
            m_hBrush = other.m_hBrush;
            other.m_hBrush = nullptr;
            return *this;
        }

        ~GdiBrush() { Reset(); }

        void Reset()
        {
            if (m_hBrush)
            {
                ::DeleteObject(m_hBrush);
                m_hBrush = nullptr;
            }
        }

        HBRUSH Get() const { return m_hBrush; }
        operator HBRUSH() const { return m_hBrush; }

        static GdiBrush Solid(COLORREF color)
        {
            return GdiBrush(::CreateSolidBrush(color));
        }
    };

    // 普通槽：返回值 void（command/mouse/key等绝大多数信号）
    template <typename... Args>
    struct SlotWrapper
    {
        SlotHandle handle;
        std::function<void(Args...)> fn;

        SlotWrapper(SlotHandle h, std::function<void(Args...)> f)
            : handle(h), fn(std::move(f)) {}
    };

    // 特殊 WndProc槽：返回LRESULT，专门给signal_wndProc
    struct WndProcSlotWrapper
    {
        SlotHandle handle;
        std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fn;
        WndProcSlotWrapper(SlotHandle h, std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> f)
            : handle(h), fn(std::move(f)) {}
    };

    // 普通信号容器，返回void
    template <typename... Args>
    struct Signal
    {
        std::vector<SlotWrapper<Args...>> slots;
        bool blocked = false;
    };

    // WndProc专用信号容器，返回LRESULT
    struct WndProcSignal
    {
        std::vector<WndProcSlotWrapper> slots;
        bool blocked = false;
    };

    // ===================== ScopedConnection 普通信号 RAII =====================
    template <typename TObj, typename TSignalMemPtr>
    class ScopedConnection
    {
    private:
        TObj *m_obj = nullptr;
        TSignalMemPtr m_sigMem{};
        SlotHandle m_handle = 0;

    public:
        ScopedConnection() = default;
        ScopedConnection(TObj *obj, TSignalMemPtr sigMem, SlotHandle h)
            : m_obj(obj), m_sigMem(sigMem), m_handle(h) {}

        ScopedConnection(const ScopedConnection &) = delete;
        ScopedConnection &operator=(const ScopedConnection &) = delete;

        ScopedConnection(ScopedConnection &&other) noexcept
        {
            m_obj = other.m_obj;
            m_sigMem = other.m_sigMem;
            m_handle = other.m_handle;
            other.m_obj = nullptr;
            other.m_handle = 0;
        }
        ScopedConnection &operator=(ScopedConnection &&other) noexcept
        {
            reset();
            m_obj = other.m_obj;
            m_sigMem = other.m_sigMem;
            m_handle = other.m_handle;
            other.m_obj = nullptr;
            other.m_handle = 0;
            return *this;
        }

        void reset()
        {
            if (m_obj && m_handle != 0)
            {
                disconnect(m_obj, m_sigMem, m_handle);
                m_handle = 0;
                m_obj = nullptr;
            }
        }

        SlotHandle release()
        {
            SlotHandle h = m_handle;
            m_obj = nullptr;
            m_handle = 0;
            return h;
        }

        ~ScopedConnection() { reset(); }
        bool valid() const { return m_obj != nullptr && m_handle != 0; }
    };

    // ===================== ScopedConnectionWndProc WndProc信号RAII =====================
    template <typename TObj>
    class ScopedConnectionWndProc
    {
    private:
        TObj *m_obj = nullptr;
        WndProcSignal TObj::*m_sigMem{};
        SlotHandle m_handle = 0;

    public:
        ScopedConnectionWndProc() = default;
        ScopedConnectionWndProc(TObj *obj, WndProcSignal TObj::*sigMem, SlotHandle h)
            : m_obj(obj), m_sigMem(sigMem), m_handle(h) {}

        ScopedConnectionWndProc(const ScopedConnectionWndProc &) = delete;
        ScopedConnectionWndProc &operator=(const ScopedConnectionWndProc &) = delete;

        ScopedConnectionWndProc(ScopedConnectionWndProc &&other) noexcept
        {
            m_obj = other.m_obj;
            m_sigMem = other.m_sigMem;
            m_handle = other.m_handle;
            other.m_obj = nullptr;
            other.m_handle = 0;
        }
        ScopedConnectionWndProc &operator=(ScopedConnectionWndProc &&other) noexcept
        {
            reset();
            m_obj = other.m_obj;
            m_sigMem = other.m_sigMem;
            m_handle = other.m_handle;
            other.m_obj = nullptr;
            other.m_handle = 0;
            return *this;
        }

        void reset()
        {
            if (m_obj && m_handle != 0)
            {
                disconnectWndProc(m_obj, m_sigMem, m_handle);
                m_handle = 0;
                m_obj = nullptr;
            }
        }
        SlotHandle release()
        {
            SlotHandle h = m_handle;
            m_obj = nullptr;
            m_handle = 0;
            return h;
        }
        ~ScopedConnectionWndProc() { reset(); }
        bool valid() const { return m_obj != nullptr && m_handle != 0; }
    };

    // 普通信号工厂 connectScoped
    template <typename TObj, typename TSignalMemPtr, typename TSlot>
    ScopedConnection<TObj, TSignalMemPtr> connectScoped(TObj *obj, TSignalMemPtr signalMem, TSlot &&slot)
    {
        auto h = connect(obj, signalMem, std::forward<TSlot>(slot));
        return ScopedConnection<TObj, TSignalMemPtr>(obj, signalMem, h);
    }

    template <typename TObj, typename TSignalMemPtr, typename TReceiver, typename TMemFn>
    ScopedConnection<TObj, TSignalMemPtr> connectScoped(TObj *obj, TSignalMemPtr signalMem, TReceiver *receiver, TMemFn memFn)
    {
        auto h = connect(obj, signalMem, receiver, memFn);
        return ScopedConnection<TObj, TSignalMemPtr>(obj, signalMem, h);
    }

    // WndProc信号工厂 connectScopedWndProc
    template <typename TObj, typename TSlot>
    ScopedConnectionWndProc<TObj> connectScopedWndProc(TObj *obj, WndProcSignal TObj::*sigMem, TSlot &&slot)
    {
        auto h = connectWndProc(obj, sigMem, std::forward<TSlot>(slot));
        return ScopedConnectionWndProc<TObj>(obj, sigMem, h);
    }

    // ==========================================

    class WinGUI
    {
    private:
        HWND m_hWnd;
        std::wstring m_className;
        std::wstring m_windowTitle;
        int m_width;
        int m_height;
        HINSTANCE m_hInstance;

    public:
        WndProcSignal signal_wndProc; // 特殊，带LRESULT返回值
        Signal<int, int, HWND> signal_command;
        Signal<HWND> signal_wndClose;
        Signal<HWND> signal_wndDestroy;
        Signal<HWND, HDC> signal_wndPaint;
        Signal<UINT_PTR> signal_timer;
        Signal<HWND, int, int, UINT> signal_mouseMove;
        Signal<HWND, int, int, UINT> signal_lButtonDown;
        Signal<HWND, int, int, UINT> signal_rButtonDown;
        Signal<HWND, int, int, UINT> signal_lButtonUp;
        Signal<HWND, int, int, UINT> signal_rButtonUp;
        Signal<HWND, UINT, UINT, UINT> signal_keyDown;
        Signal<HWND, UINT, int, int> signal_wndSize;
        Signal<HWND, const std::vector<std::wstring> &> signal_dropFiles;
        Signal<HWND, int, int, int, UINT> signal_mouseWheel;
        Signal<HWND, HWND> signal_captureChanged;

    private:
        static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        bool RegisterWindowClass();

    public:
        enum ControlID
        {
            ID_BUTTON_BASE = 1000,
            ID_EDIT_BASE = 2000,
            ID_STATIC_BASE = 3000,
            ID_LISTBOX_BASE = 4000,
            ID_MENU_BASE = 5000,
            ID_CHECKBOX_BASE = 6000,
            ID_RADIO_BASE = 7000,
            ID_COMBO_BASE = 8000
        };

        WinGUI(const std::wstring &clsName = L"WinGUI_Class",
               const std::wstring &title = L"WinGUI窗口",
               int w = 800, int h = 600);
        ~WinGUI();

        bool Create(HINSTANCE hInst, int nCmdShow);
        int RunMessageLoop();

        UINT_PTR StartTimer(UINT_PTR timerId, UINT elapseMs);
        void StopTimer(UINT_PTR timerId);
        void EnableFileDrop(bool enable);

        HWND CreateButtonCtrl(const std::wstring &text, int x, int y, int w, int h, int id);
        HWND CreateStaticTextCtrl(const std::wstring &text, int x, int y, int w, int h, int id);
        HWND CreateEditBoxCtrl(int x, int y, int w, int h, int id,
                               bool isMultiLine = false, bool isPassword = false, bool isReadOnly = false);
        HWND CreateListBoxCtrl(int x, int y, int w, int h, int id, bool isSort = true);

        HWND CreateCheckBoxCtrl(const std::wstring &text, int x, int y, int w, int h, int id);
        HWND CreateRadioButtonCtrl(const std::wstring &text, int x, int y, int w, int h, int id);
        HWND CreateComboBoxCtrl(int x, int y, int w, int h, int id, bool isDropList = false);

        bool SetCheckBoxCheck(HWND hCtrl, bool checked);
        bool GetCheckBoxCheck(HWND hCtrl);

        int ComboBoxAddItem(HWND hCombo, const std::wstring &text);
        void ComboBoxDeleteSelected(HWND hCombo);
        int ComboBoxGetSelectedIndex(HWND hCombo);
        bool ComboBoxSetSelectedIndex(HWND hCombo, int idx);

        bool SetCtrlText(HWND hCtrl, const std::wstring &text);
        std::wstring GetCtrlText(HWND hCtrl);

        int ListBoxAddItemCtrl(HWND hListBox, const std::wstring &text);
        void ListBoxDeleteSelectedCtrl(HWND hListBox);

        HMENU CreateMainMenuCtrl(const std::vector<std::pair<std::wstring,
                                                             std::vector<std::pair<std::wstring, int>>>> &menuItems);

        void ShowErrorMsg(const std::wstring &msg, DWORD errorCode = 0);
        int ShowInfoMsg(const std::wstring &msg, const std::wstring &title = L"提示");
        bool ShowConfirmMsg(const std::wstring &msg, const std::wstring &title = L"确认");

        std::wstring OpenFileDialogCtrl(const std::wstring &filter = L"所有文件 (*.*)\0*.*\0文本文件 (*.txt)\0*.txt\0");
        std::wstring SaveFileDialogCtrl(const std::wstring &filter = L"文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0");

        void SetWndTitle(const std::wstring &title);
        void MoveResizeWindow(int x, int y, int w, int h, bool repaint = true);
        void CenterWnd();
        void ShowHideWnd(bool show);
        void EnableDisableWnd(bool enable);

        HDC GetWindowDCHandle();
        void ReleaseWindowDCHandle(HDC hdc);
        void DrawTextToWnd(const std::wstring &text, int x, int y, COLORREF color = RGB(0, 0, 0));
        void DrawRectToWnd(int x, int y, int w, int h, COLORREF color = RGB(0, 0, 0), bool fill = false);

        HWND GetWndHandle() const { return m_hWnd; }
        HINSTANCE GetInstHandle() const { return m_hInstance; }
    };

    // ========== 普通信号 connect/disconnect ==========
    template <typename TObj, typename TSignalMemPtr, typename TSlot>
    SlotHandle connect(TObj *obj, TSignalMemPtr signalMem, TSlot &&slot)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        using SlotFunc = typename SigType::value_type::fn_type;
        SlotHandle h = NextSlotHandle();
        SigType &sig = std::invoke(signalMem, obj);
        sig.slots.emplace_back(h, SlotFunc(std::forward<TSlot>(slot)));
        return h;
    }

    template <typename TObj, typename TSignalMemPtr, typename TReceiver, typename TMemFn>
    SlotHandle connect(TObj *obj, TSignalMemPtr signalMem, TReceiver *receiver, TMemFn memFn)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        using SlotFunc = typename SigType::value_type::fn_type;
        SlotHandle h = NextSlotHandle();
        SigType &sig = std::invoke(signalMem, obj);
        auto boundFn = [receiver, memFn](auto &&...args)
        {
            (receiver->*memFn)(std::forward<decltype(args)>(args)...);
        };
        sig.slots.emplace_back(h, SlotFunc(std::move(boundFn)));
        return h;
    }

    template <typename TObj, typename TSignalMemPtr>
    void disconnect(TObj *obj, TSignalMemPtr signalMem, SlotHandle handle)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        SigType &sig = std::invoke(signalMem, obj);
        for (auto it = sig.slots.begin(); it != sig.slots.end();)
        {
            if (it->handle == handle)
                it = sig.slots.erase(it);
            else
                ++it;
        }
    }

    template <typename TObj, typename TSignalMemPtr>
    void disconnectAll(TObj *obj, TSignalMemPtr signalMem)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        SigType &sig = std::invoke(signalMem, obj);
        sig.slots.clear();
    }

    template <typename TObj, typename TSignalMemPtr>
    void blockSignal(TObj *obj, TSignalMemPtr signalMem, bool blocked)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        SigType &sig = std::invoke(signalMem, obj);
        sig.blocked = blocked;
    }

    template <typename TObj, typename TSignalMemPtr>
    bool isSignalBlocked(TObj *obj, TSignalMemPtr signalMem)
    {
        using SigType = decltype(std::invoke(signalMem, obj));
        SigType &sig = std::invoke(signalMem, obj);
        return sig.blocked;
    }

    // ========== WndProcSignal 专用接口（signal_wndProc） ==========
    template <typename TObj, typename TSlot>
    SlotHandle connectWndProc(TObj *obj, WndProcSignal TObj::*sigMem, TSlot &&slot)
    {
        using FuncT = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;
        SlotHandle h = NextSlotHandle();
        WndProcSignal &sig = obj->*sigMem;
        sig.slots.emplace_back(h, FuncT(std::forward<TSlot>(slot)));
        return h;
    }

    template <typename TObj>
    void disconnectWndProc(TObj *obj, WndProcSignal TObj::*sigMem, SlotHandle handle)
    {
        WndProcSignal &sig = obj->*sigMem;
        for (auto it = sig.slots.begin(); it != sig.slots.end();)
        {
            if (it->handle == handle)
                it = sig.slots.erase(it);
            else
                ++it;
        }
    }

    template <typename TObj>
    void disconnectAllWndProc(TObj *obj, WndProcSignal TObj::*sigMem)
    {
        WndProcSignal &sig = obj->*sigMem;
        sig.slots.clear();
    }

    template <typename TObj>
    void blockWndProcSignal(TObj *obj, WndProcSignal TObj::*sigMem, bool blocked)
    {
        WndProcSignal &sig = obj->*sigMem;
        sig.blocked = blocked;
    }

    template <typename TObj>
    bool isWndProcSignalBlocked(TObj *obj, WndProcSignal TObj::*sigMem)
    {
        WndProcSignal &sig = obj->*sigMem;
        return sig.blocked;
    }

} // namespace SyLib

#endif // SYGUI_H
#else
#error "This OS isn't Windows!"
#endif