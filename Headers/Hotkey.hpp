#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <vector>
#include <string>

struct Hotkey
{
    std::vector<sf::Keyboard::Scancode> keys;
    
    Hotkey() = default;
    Hotkey(const std::vector<sf::Keyboard::Scancode>& ks);
    
    bool isTriggered(const sf::Event::KeyPressed* kp) const;
    std::string toString() const;
    void fromString(const std::string& str);
};
