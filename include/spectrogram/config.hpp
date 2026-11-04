#pragma once

#include <SFML/Window.hpp>

struct Config {
    sf::Vector2u initial_winsize = {1280, 720};
    sf::Vector2u min_winsize = {640, 480};
    sf::Vector2u no_sidebar_winsize = {768, 432};
    bool auto_maximize = true;
    bool collapse_sidebar = false;
    float sidebar_width = 250.f;
    sf::Vector2f sidebar_toggle_size = {30.f, 30.f};
    float statusbar_height = 40.f;
    
    sf::Vector2u spectogram_resolution = {512, 512};
};
