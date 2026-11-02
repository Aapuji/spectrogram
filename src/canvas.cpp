#include <spectogram/canvas.hpp>

#include <iostream>
#include <ranges> 

#include <TGUI/TGUI.hpp>

SpectrogramCanvas::SpectrogramCanvas(sf::Vector2u data_size, RingBuffer<SpectrumFrame>& fft_buf)
    : m_size{data_size},
      m_fft_buf{fft_buf},
      m_texture_buf{m_size.x},
      m_head{0},
      m_texture{data_size},
      m_sprite{m_texture},
      m_canvas{tgui::CanvasSFML::create({
        static_cast<float>(data_size.x),
        static_cast<float>(data_size.y)
      })}
{
    m_texture.setSmooth(true);
    m_texture.setRepeated(true);
}

void SpectrogramCanvas::update_layout(sf::Vector2f pos, sf::Vector2f size) {
    // m_sprite.setPosition({0, 0});

    if (m_size.x > 0 && m_size.y > 0) {
        m_sprite.setScale({
            size.x / static_cast<float>(m_size.x),
            size.y / static_cast<float>(m_size.y)
        });
    }
}

void SpectrogramCanvas::draw() {
    m_canvas->clear(sf::Color::Black);
    m_canvas->draw(m_sprite);
    m_canvas->display();
}

void SpectrogramCanvas::update_plot() {
    SpectrumFrame frame;
    
    auto colors = m_fft_buf.pop(frame)
        ? color_frame(frame)
        : color_frame();

    m_texture.update(
        colors.data(),
        {1, static_cast<unsigned int>(frame.bins.size())},
        {m_head, 0}
    );
    
    m_head = (m_head + 1) % m_size.x;
    m_sprite.setTextureRect(sf::IntRect{
        {static_cast<int>(m_head), 0},
        {static_cast<int>(m_size.x), static_cast<int>(m_size.y)}
    });
}

std::vector<uint8_t> SpectrogramCanvas::color_frame(const SpectrumFrame& frame) const {
    std::vector<uint8_t> colors;
    colors.reserve(frame.bins.size() * 4);

    for (const auto& bin : frame.bins | std::views::reverse) {
        colors.push_back(static_cast<uint8_t>(bin * 255));
        colors.push_back(0);
        colors.push_back(0);
        colors.push_back(255);
    }

    return colors;
}

std::vector<uint8_t> SpectrogramCanvas::color_frame() const {
    std::vector<uint8_t> colors;
    colors.reserve(255 * 4);

    for (int i = 254; i >= 0; i--) {
        colors.push_back(0);
        colors.push_back(0);
        colors.push_back(0);
        colors.push_back(255);
    }

    return colors;
}

tgui::CanvasSFML::Ptr SpectrogramCanvas::widget() const {
    return m_canvas;
}
