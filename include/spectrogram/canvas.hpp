#pragma once

#include <string_view>
#include <vector>

#include <SFML/Graphics.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>

#include "frame.hpp"
#include "ring_buffer.hpp"

class SpectrogramCanvas {
private:
    sf::Vector2u m_size; // size of the texture, not sprite
    RingBuffer<SpectrumFrame>& m_fft_buf;
    std::vector<SpectrumFrame> m_texture_buf;
    unsigned int m_head;
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    tgui::CanvasSFML::Ptr m_canvas;

    std::vector<uint8_t> color_frame(const SpectrumFrame& frame) const;
    std::vector<uint8_t> color_frame() const;
public:
    SpectrogramCanvas(sf::Vector2u data_size, RingBuffer<SpectrumFrame>& fft_buf);
    void update_layout(sf::Vector2f pos, sf::Vector2f size);

    /// @brief Only draws canvas to window. Does not update spectogram.
    void draw();

    /// @brief Only updates spectogram.
    void update_plot();

    unsigned int num_available() const {
        return m_fft_buf.num_ready_to_read();
    }

    tgui::CanvasSFML::Ptr widget() const;
};
