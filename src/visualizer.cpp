#include <spectrogram/visualizer.hpp>
#include <spectrogram/platform.hpp>
#include <iostream>

Visualizer::Visualizer(const Config& config, RingBuffer<SpectrumFrame>& buf)
    : m_window{sf::VideoMode(config.initial_winsize), "Spectogram"},
      m_canvas{config.spectogram_resolution, buf},
      m_gui{m_window},
      m_is_sidebar_open{!config.collapse_sidebar},
      m_sidebar_width{config.sidebar_width},
      m_statusbar_height{config.statusbar_height},
      m_no_sidebar_winsize{config.no_sidebar_winsize}
{
    m_window.setMinimumSize(config.min_winsize);
    m_window.setVerticalSyncEnabled(true);
    init(config);
}

void Visualizer::init(const Config& config) {
    if (config.auto_maximize)
        maximize();
    
    m_statusbar = tgui::Label::create();
    m_statusbar->setText("Text tbd");
    m_statusbar->setPosition({0, tgui::bindHeight(m_gui) - m_statusbar_height});
    m_statusbar->setSize({tgui::bindWidth(m_gui), m_statusbar_height});
    m_gui.add(m_statusbar);

    m_sidebar = tgui::Panel::create();
    m_sidebar->setPosition({tgui::bindWidth(m_gui) - m_sidebar_width, 0});
    m_sidebar->setSize({m_sidebar_width, tgui::bindHeight(m_gui) - tgui::bindHeight(m_statusbar)});
    m_gui.add(m_sidebar);

    m_canvas.widget()->setPosition({0, 0});
    m_canvas.widget()->setSize({tgui::bindLeft(m_sidebar), tgui::bindTop(m_statusbar)});
    m_gui.add(m_canvas.widget());

    m_sidebar_toggle = tgui::Button::create();
    m_sidebar_toggle->setSize(static_cast<tgui::Layout2d>(config.sidebar_toggle_size));
    m_sidebar_toggle->onPress(&Visualizer::toggle_sidebar, this);
    m_gui.add(m_sidebar_toggle);

    platform::install_live_resize_hook(m_window.getNativeHandle(), [this] { on_native_resize(); });
    m_ready = true;

    update_layout();
}

void Visualizer::loop() {
    while (m_window.isOpen()) {
        while (const std::optional event = m_window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                m_window.close();
            
            handle_event(*event);
        }
        
        std::cout << m_canvas.num_available() << std::endl;
        draw_frame();
    }
}

void Visualizer::draw_frame() {
    m_window.clear(sf::Color::Black);
    render();
    m_window.display();
}

void Visualizer::render() {
    m_canvas.update_plot();
    m_canvas.draw();
    m_gui.draw();
}

void Visualizer::update_layout() {
    const sf::Vector2u winsize = m_window.getSize();

    const float sidebar_width = m_is_sidebar_open && !m_is_sidebar_hidden ? m_sidebar_width : 0.f;

    m_sidebar->setPosition({tgui::bindWidth(m_gui) - sidebar_width, 0});
    m_sidebar->setSize({sidebar_width, tgui::bindHeight(m_gui) - tgui::bindHeight(m_statusbar)});
    
    m_canvas.widget()->setPosition({0, 0});
    m_canvas.widget()->setSize({tgui::bindLeft(m_sidebar), tgui::bindTop(m_statusbar)});

    // const tgui::Layout2d canvas_size = {tgui::bindLeft(m_sidebar), tgui::bindTop(m_statusbar)};
    m_canvas.update_layout({0, 0}, m_canvas.widget()->getSize());

    const tgui::Layout2d sidebar_toggle_offset = {m_is_sidebar_open ? 10.f : -tgui::bindWidth(m_sidebar_toggle) - 10.f, 10.f};
    m_sidebar_toggle->setPosition(tgui::Layout2d{tgui::bindLeft(m_sidebar), tgui::bindTop(m_sidebar)} + sidebar_toggle_offset);
    
    m_statusbar->setPosition({0, tgui::bindHeight(m_gui) - m_statusbar_height});
    m_statusbar->setSize({tgui::bindWidth(m_gui), m_statusbar_height});
}

void Visualizer::handle_event(const sf::Event& event) {
    m_gui.handleEvent(event);

    if (const auto* resized = event.getIf<sf::Event::Resized>()) {
        const auto new_size = resized->size;

        if (new_size.x <= m_no_sidebar_winsize.x)
            disable_sidebar();
        else
            enable_sidebar();
        
        m_window.setView(sf::View(sf::FloatRect({0.f, 0.f}, sf::Vector2f(new_size))));
        
        update_layout();
    } else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Q && key->control) {
            m_window.close();
        }
    }
}

void Visualizer::toggle_sidebar() {
    m_is_sidebar_open  = !m_is_sidebar_open;
    m_sidebar->setVisible(m_is_sidebar_open);
    
    update_layout();
}

void Visualizer::disable_sidebar() {
    m_is_sidebar_hidden = true;
    m_sidebar->setVisible(false);
    m_sidebar_toggle->setVisible(false);

    update_layout();
}

void Visualizer::enable_sidebar() {
    m_is_sidebar_hidden = false;
    m_sidebar_toggle->setVisible(true);

    if (m_is_sidebar_open)
        m_sidebar->setVisible(true);
}

void Visualizer::maximize() {
    platform::maximize_window(m_window.getNativeHandle());
}

void Visualizer::on_native_resize() {
    if (!m_ready || m_in_native_resize || !m_window.isOpen())
        return;

    m_in_native_resize = true;

    // The hook just made SFML queue a Resized event, so this pops it without pumping messages
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

        handle_event(*event);

        if (event->is<sf::Event::Resized>())
            break;
    }

    if (m_window.isOpen())
        draw_frame();

    m_in_native_resize = false;
}

Visualizer::~Visualizer() {
    if (m_window.isOpen())
        platform::remove_live_resize_hook(m_window.getNativeHandle());
}
