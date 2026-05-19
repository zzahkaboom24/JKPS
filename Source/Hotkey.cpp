#include "../Headers/Hotkey.hpp"
#include "../Headers/StringHelper.hpp"

Hotkey::Hotkey(const std::vector<sf::Keyboard::Scancode>& ks) : keys(ks) {}

bool Hotkey::isTriggered(const sf::Event::KeyPressed* kp) const
{
    if (keys.empty()) return false;
    
    // The trigger key is the last key in the combination
    if (kp->scancode != keys.back()) return false;
    
    // Check if all modifier keys are held down
    for (size_t i = 0; i < keys.size() - 1; ++i)
    {
        if (!sf::Keyboard::isKeyPressed(keys[i])) 
            return false;
    }
    return true;
}

std::string Hotkey::toString() const
{
    if (keys.empty()) return "None";
    std::string res;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        res += scancodeToStr(keys[i], true);
        if (i < keys.size() - 1) res += " + ";
    }
    return res;
}

void Hotkey::fromString(const std::string& str)
{
    keys.clear();
    if (str == "None" || str.empty()) return;
    
    size_t start = 0;
    size_t end = str.find(" + ");
    while (end != std::string::npos)
    {
        std::string part = str.substr(start, end - start);
        keys.push_back(strToScancode(part));
        start = end + 3;
        end = str.find(" + ", start);
    }
    std::string last = str.substr(start);
    keys.push_back(strToScancode(last));
}
