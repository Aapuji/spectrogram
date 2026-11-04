#pragma once

#include <functional>
#include <SFML/Window.hpp>

namespace platform {
    /// @brief Maximizes the window using the native API.
    void maximize_window(sf::WindowHandle handle);

    /// @brief Calls `on_resize` callback on every size change, including mid-drag.
    void install_live_resize_hook(sf::WindowHandle handle, std::function<void()> on_resize);
    void remove_live_resize_hook(sf::WindowHandle handle);

}
