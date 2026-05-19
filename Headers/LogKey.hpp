#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <string>

struct LogKey
{
    // Migrated pointers from sf::Keyboard::Key to sf::Keyboard::Scancode
    LogKey(const std::string& realStr, const std::string& visualStr, sf::Keyboard::Scancode *key, sf::Mouse::Button *button);

    bool isPressed() const;
    bool resetChangedState();

    std::string realStr;
    std::string visualStr;
    sf::Keyboard::Scancode *keyboardKey; // Migrated to Scancode
    sf::Mouse::Button *mouseButton;
    bool changed;
};