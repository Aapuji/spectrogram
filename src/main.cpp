#include <iostream>

#include <spectogram/visualizer.hpp>

int main() {
    Config config{};
    RingBuffer<SpectrumFrame> fftgui_buf{512};

    const float INCR_512 = 1.f / 512;
    for (auto x = 0; x < 512; x++) {
        std::vector<float> frame_data;
        frame_data.reserve(255);

        for (auto y = 0; y < 512; y++) {
            frame_data.push_back(y * INCR_512);
        }

        SpectrumFrame frame{frame_data};
        fftgui_buf.push(std::move(frame));
    }

    std::cout << fftgui_buf.num_ready_to_read() << std::endl;

    Visualizer vis{config, fftgui_buf};

    vis.loop();
}
