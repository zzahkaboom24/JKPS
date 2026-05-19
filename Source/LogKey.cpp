#include "../Headers/LogKey.hpp"

LogKey::LogKey(const std::string& realStr, const std::string& visualStr, sf::Keyboard::Scancode *key, sf::Mouse::Button *button)
    : realStr(realStr)
    , visualStr(visualStr)
    , keyboardKey(key)
    , mouseButton(button)
    , changed(false)
{
}

bool LogKey::isPressed() const
{
    if (keyboardKey && *keyboardKey != sf::Keyboard::Scancode::Unknown)
    {
        return sf::Keyboard::isKeyPressed(*keyboardKey);
    }
    if (mouseButton)
    {
        return sf::Mouse::isButtonPressed(*mouseButton);
    }
    return false;
}

bool LogKey::resetChangedState()
{
    bool oldState = changed;
    changed = false;
    return oldState;
}