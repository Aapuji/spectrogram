#include <spectrogram/platform.hpp>

#include <windows.h>

namespace {
    struct Hook {
        WNDPROC original;
        std::function<void()> callback;
        bool in_drag = false;   // inside the modal resize/move loop
    };


    // SFML already uses GWLP_USERDATA for its own pointer, so use a window property.
    constexpr const wchar_t* kProp = L"spectogram_resize_hook";

    void uninstall(HWND hwnd) {
        auto* hook = static_cast<Hook*>(GetPropW(hwnd, kProp));
        if (!hook) return;
        SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hook->original));
        RemovePropW(hwnd, kProp);
        delete hook;
    }

    LRESULT CALLBACK proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        auto* hook = static_cast<Hook*>(GetPropW(hwnd, kProp));
        if (!hook)
            return DefWindowProcW(hwnd, msg, wp, lp);

        const WNDPROC original = hook->original;  // copy: hook may be deleted below

        switch (msg) {
            case WM_NCDESTROY:
                uninstall(hwnd);
                return CallWindowProcW(original, hwnd, msg, wp, lp);
            case WM_ENTERSIZEMOVE:
                hook->in_drag = true;
                break;
            case WM_EXITSIZEMOVE:
                hook->in_drag = false;
                break;
        }

        const LRESULT result = CallWindowProcW(original, hwnd, msg, wp, lp);

        if (msg == WM_SIZE && wp != SIZE_MINIMIZED && hook->in_drag) {
            // SFML holds back Resized events until the drag ends. Tell it the drag
            // "ended" so it queues one for the current size, then re-arm it.
            CallWindowProcW(original, hwnd, WM_EXITSIZEMOVE, 0, 0);
            CallWindowProcW(original, hwnd, WM_ENTERSIZEMOVE, 0, 0);
            hook->callback();
        }

        return result;
    }
}

void platform::maximize_window(sf::WindowHandle handle) {
    ShowWindow(handle, SW_MAXIMIZE);
}

void platform::install_live_resize_hook(sf::WindowHandle handle, std::function<void()> on_resize) {
    if (GetPropW(handle, kProp)) return;  // already installed

    auto* hook = new Hook{nullptr, std::move(on_resize)};
    hook->original = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(handle, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(proc)));
    SetPropW(handle, kProp, hook);
}

void platform::remove_live_resize_hook(sf::WindowHandle handle) {
    uninstall(handle);
}
