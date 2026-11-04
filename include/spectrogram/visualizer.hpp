#pragma once

#include <complex>
#include <vector>

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "canvas.hpp"
#include "config.hpp"
#include "frame.hpp"
#include "ring_buffer.hpp"

class Visualizer {
private:
    sf::RenderWindow m_window;
    SpectrogramCanvas m_canvas;

    tgui::Gui m_gui;
    tgui::Panel::Ptr m_sidebar;
    tgui::Button::Ptr m_sidebar_toggle;
    tgui::Label::Ptr m_statusbar;
    bool m_is_sidebar_open;
    float m_sidebar_width;
    float m_statusbar_height;
    bool m_ready = false; // true once init() has built all widgets
    bool m_in_native_resize = false;
    sf::Vector2u m_no_sidebar_winsize;
    bool m_is_sidebar_hidden = false;

    void init(const Config& config);
    void maximize();
    void update_layout();
    void draw_frame();
    void render(); // TODO: use SpectrumFrame's instead
    void toggle_sidebar();
    void handle_event(const sf::Event& event);
    void disable_sidebar();
    void enable_sidebar();
    
    /// @brief Called from the native window proc while the user drags a resize edge. 
    /// 
    /// Needed because there isn't a way to cross-platform render/change layout while the window is resizing. Only afterwards.
    void on_native_resize();
public:
    Visualizer(const Config& config, RingBuffer<SpectrumFrame>& buf);
    ~Visualizer();

    Visualizer(const Visualizer&) = delete;
    Visualizer& operator=(const Visualizer&) = delete;

    Visualizer(Visualizer&& other) noexcept = default;
    Visualizer& operator=(Visualizer&& other) noexcept = default;

    void loop();
};
