#include "../Headers/StringHelper.hpp"
#include "../Headers/LogKey.hpp"

#include <SFML/System/String.hpp>
#include <cassert>


unsigned readAmountOfParms(const std::string &str)
{
    unsigned amt = 0;
    for (unsigned idx = 0; idx < str.size(); ++idx)
    {
        if (str[idx] == ',' || idx + 1 == str.size())
            ++amt;
    }

    return amt;
}

std::string readValue(const std::string &str, unsigned n)
{
    unsigned idx, nVal = 0;
    // Skip all previous values
    for (idx = 0; idx < str.size() && nVal != n; ++idx)
    {
        if (str[idx] == ',')
            ++nVal;
    }

    assert(nVal == n);

    std::string retVal;
    // Write everything in new str until newline or comma
    for (; idx < str.size() && str[idx] != ','; ++idx)
    {
        retVal += str[idx];
    }

    return retVal;
}

static void modifyNumOnIdx(std::string &str, unsigned idx, bool add, char num = ' ')
{
    std::string substr = str.substr(idx + !add);
    if (add)
    {
        str.resize(str.size() + 1);
        str[idx] = num;
        for (unsigned i = 0; i < substr.size(); ++i)
            str[idx + i + 1] = substr[i];
    }
    else
    {
        for (unsigned i = 0; i < substr.size(); ++i)
            str[idx + i] = substr[i];
        str.resize(str.size() - 1);
    }
}

void addChOnIdx(std::string &str, unsigned idx, char ch)
{
    modifyNumOnIdx(str, idx, true, ch);
}

void rmChOnIdx(std::string &str, unsigned idx)
{
    modifyNumOnIdx(str, idx, false);
}

std::string eraseDigitsOverHundredths(const std::string &floatStr)
{
    std::size_t pointIdx;
    const bool isFloat = (pointIdx = floatStr.find('.')) != std::string::npos;
    return isFloat ? floatStr.substr(0, pointIdx + 2) : floatStr;
}


// For more information
// https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Keyboard.html
// https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1Mouse.html
// sf::Keyboard::Key::A is used as unknown key in order to make the user change the key
// regardless of the fact that they didn't pass the idiot proof.
// Same thing with mouse: sf::Mouse::Button::Left as unknown button.


std::string keyToStr(sf::Keyboard::Key key, bool saveToCfg)
{
    switch (key)
    {
        case sf::Keyboard::Key::A: return "A";
        case sf::Keyboard::Key::B: return "B";
        case sf::Keyboard::Key::C: return "C";
        case sf::Keyboard::Key::D: return "D";
        case sf::Keyboard::Key::E: return "E";
        case sf::Keyboard::Key::F: return "F";
        case sf::Keyboard::Key::G: return "G";
        case sf::Keyboard::Key::H: return "H";
        case sf::Keyboard::Key::I: return "I";
        case sf::Keyboard::Key::J: return "J";
        case sf::Keyboard::Key::K: return "K";
        case sf::Keyboard::Key::L: return "L";
        case sf::Keyboard::Key::M: return "M";
        case sf::Keyboard::Key::N: return "N";
        case sf::Keyboard::Key::O: return "O";
        case sf::Keyboard::Key::P: return "P";
        case sf::Keyboard::Key::Q: return "Q";
        case sf::Keyboard::Key::R: return "R";
        case sf::Keyboard::Key::S: return "S";
        case sf::Keyboard::Key::T: return "T";
        case sf::Keyboard::Key::U: return "U";
        case sf::Keyboard::Key::V: return "V";
        case sf::Keyboard::Key::W: return "W";
        case sf::Keyboard::Key::X: return "X";
        case sf::Keyboard::Key::Y: return "Y";
        case sf::Keyboard::Key::Z: return "Z";
        case sf::Keyboard::Key::Num0: return (saveToCfg ? "Num0" : "0");
        case sf::Keyboard::Key::Num1: return (saveToCfg ? "Num1" : "1");
        case sf::Keyboard::Key::Num2: return (saveToCfg ? "Num2" : "2");
        case sf::Keyboard::Key::Num3: return (saveToCfg ? "Num3" : "3");
        case sf::Keyboard::Key::Num4: return (saveToCfg ? "Num4" : "4");
        case sf::Keyboard::Key::Num5: return (saveToCfg ? "Num5" : "5");
        case sf::Keyboard::Key::Num6: return (saveToCfg ? "Num6" : "6");
        case sf::Keyboard::Key::Num7: return (saveToCfg ? "Num7" : "7");
        case sf::Keyboard::Key::Num8: return (saveToCfg ? "Num8" : "8");
        case sf::Keyboard::Key::Num9: return (saveToCfg ? "Num9" : "9");
        case sf::Keyboard::Key::Escape: return "Escape";
        case sf::Keyboard::Key::LControl: return "LControl";
        case sf::Keyboard::Key::LShift: return "LShift";
        case sf::Keyboard::Key::LAlt: return "LAlt";
        case sf::Keyboard::Key::LSystem: return "LSystem";
        case sf::Keyboard::Key::RControl: return "RControl";
        case sf::Keyboard::Key::RShift: return "RShift";
        case sf::Keyboard::Key::RAlt: return "RAlt";
        case sf::Keyboard::Key::RSystem: return "RSystem";
        case sf::Keyboard::Key::Menu: return "Menu";
        case sf::Keyboard::Key::LBracket: return (saveToCfg ? "LBracket" : "[");
        case sf::Keyboard::Key::RBracket: return (saveToCfg ? "RBracket" : "]");
        case sf::Keyboard::Key::Semicolon: return (saveToCfg ? "Semicolon" : ";");
        case sf::Keyboard::Key::Comma: return (saveToCfg ? "Comma" : ",");
        case sf::Keyboard::Key::Period: return (saveToCfg ? "Period" : ".");
        case sf::Keyboard::Key::Apostrophe: return (saveToCfg ? "Quote" : "\""); // SFML 3: Quote -> Apostrophe
        case sf::Keyboard::Key::Slash: return (saveToCfg ? "Slash" : "/");
        case sf::Keyboard::Key::Backslash: return (saveToCfg ? "Backslash" : "\\");
        case sf::Keyboard::Key::Grave: return (saveToCfg ? "Tilde" : "~"); // SFML 3: Tilde -> Grave
        case sf::Keyboard::Key::Equal: return (saveToCfg ? "Equal" : "=");
        case sf::Keyboard::Key::Hyphen: return (saveToCfg ? "Hyphen" : "-");
        case sf::Keyboard::Key::Space: return "Space";
        case sf::Keyboard::Key::Enter: return "Enter";
        case sf::Keyboard::Key::Backspace: return "Backspace";
        case sf::Keyboard::Key::Tab: return "Tab";
        case sf::Keyboard::Key::PageUp: return (saveToCfg ? "PageUp" : "PgUp");
        case sf::Keyboard::Key::PageDown: return (saveToCfg ? "PageDown" : "PgDn");
        case sf::Keyboard::Key::End: return "End";
        case sf::Keyboard::Key::Home: return "Home";
        case sf::Keyboard::Key::Insert: return (saveToCfg ? "Insert" : "Ins");
        case sf::Keyboard::Key::Delete: return (saveToCfg ? "Delete" : "Del");
        case sf::Keyboard::Key::Add: return (saveToCfg ? "Add" : "+");
        case sf::Keyboard::Key::Subtract: return (saveToCfg ? "Subtract" : "-");
        case sf::Keyboard::Key::Multiply: return (saveToCfg ? "Multiply" : "*");
        case sf::Keyboard::Key::Divide: return (saveToCfg ? "Divide" : "/");
        case sf::Keyboard::Key::Left: return "Left";
        case sf::Keyboard::Key::Right: return "Right";
        case sf::Keyboard::Key::Up: return "Up";
        case sf::Keyboard::Key::Down: return "Down";
        case sf::Keyboard::Key::Numpad0: return (saveToCfg ? "Numpad0" : "Num0");
        case sf::Keyboard::Key::Numpad1: return (saveToCfg ? "Numpad1" : "Num1");
        case sf::Keyboard::Key::Numpad2: return (saveToCfg ? "Numpad2" : "Num2");
        case sf::Keyboard::Key::Numpad3: return (saveToCfg ? "Numpad3" : "Num3");
        case sf::Keyboard::Key::Numpad4: return (saveToCfg ? "Numpad4" : "Num4");
        case sf::Keyboard::Key::Numpad5: return (saveToCfg ? "Numpad5" : "Num5");
        case sf::Keyboard::Key::Numpad6: return (saveToCfg ? "Numpad6" : "Num6");
        case sf::Keyboard::Key::Numpad7: return (saveToCfg ? "Numpad7" : "Num7");
        case sf::Keyboard::Key::Numpad8: return (saveToCfg ? "Numpad8" : "Num8");
        case sf::Keyboard::Key::Numpad9: return (saveToCfg ? "Numpad9" : "Num9");
        case sf::Keyboard::Key::F1: return "F1";
        case sf::Keyboard::Key::F2: return "F2";
        case sf::Keyboard::Key::F3: return "F3";
        case sf::Keyboard::Key::F4: return "F4";
        case sf::Keyboard::Key::F5: return "F5";
        case sf::Keyboard::Key::F6: return "F6";
        case sf::Keyboard::Key::F7: return "F7";
        case sf::Keyboard::Key::F8: return "F8";
        case sf::Keyboard::Key::F9: return "F9";
        case sf::Keyboard::Key::F10: return "F10";
        case sf::Keyboard::Key::F11: return "F11";
        case sf::Keyboard::Key::F12: return "F12";
        case sf::Keyboard::Key::F13: return "F13";
        case sf::Keyboard::Key::F14: return "F14";
        case sf::Keyboard::Key::F15: return "F15";
        case sf::Keyboard::Key::Pause: return "Pause";
        default: return "Unknown";
    }
}

sf::Keyboard::Key strToKey(const std::string &str)
{
    if (str == "A") return sf::Keyboard::Key::A;
    if (str == "B") return sf::Keyboard::Key::B;
    if (str == "C") return sf::Keyboard::Key::C;
    if (str == "D") return sf::Keyboard::Key::D;
    if (str == "E") return sf::Keyboard::Key::E;
    if (str == "F") return sf::Keyboard::Key::F;
    if (str == "G") return sf::Keyboard::Key::G;
    if (str == "H") return sf::Keyboard::Key::H;
    if (str == "I") return sf::Keyboard::Key::I;
    if (str == "J") return sf::Keyboard::Key::J;
    if (str == "K") return sf::Keyboard::Key::K;
    if (str == "L") return sf::Keyboard::Key::L;
    if (str == "M") return sf::Keyboard::Key::M;
    if (str == "N") return sf::Keyboard::Key::N;
    if (str == "O") return sf::Keyboard::Key::O;
    if (str == "P") return sf::Keyboard::Key::P;
    if (str == "Q") return sf::Keyboard::Key::Q;
    if (str == "R") return sf::Keyboard::Key::R;
    if (str == "S") return sf::Keyboard::Key::S;
    if (str == "T") return sf::Keyboard::Key::T;
    if (str == "U") return sf::Keyboard::Key::U;
    if (str == "V") return sf::Keyboard::Key::V;
    if (str == "W") return sf::Keyboard::Key::W;
    if (str == "X") return sf::Keyboard::Key::X;
    if (str == "Y") return sf::Keyboard::Key::Y;
    if (str == "Z") return sf::Keyboard::Key::Z;
    if (str == "Num0") return sf::Keyboard::Key::Num0;
    if (str == "Num1") return sf::Keyboard::Key::Num1;
    if (str == "Num2") return sf::Keyboard::Key::Num2;
    if (str == "Num3") return sf::Keyboard::Key::Num3;
    if (str == "Num4") return sf::Keyboard::Key::Num4;
    if (str == "Num5") return sf::Keyboard::Key::Num5;
    if (str == "Num6") return sf::Keyboard::Key::Num6;
    if (str == "Num7") return sf::Keyboard::Key::Num7;
    if (str == "Num8") return sf::Keyboard::Key::Num8;
    if (str == "Num9") return sf::Keyboard::Key::Num9;
    if (str == "Escape") return sf::Keyboard::Key::Escape;
    if (str == "LControl") return sf::Keyboard::Key::LControl;
    if (str == "LShift") return sf::Keyboard::Key::LShift;
    if (str == "LAlt") return sf::Keyboard::Key::LAlt;
    if (str == "LSystem") return sf::Keyboard::Key::LSystem;
    if (str == "RControl") return sf::Keyboard::Key::RControl;
    if (str == "RShift") return sf::Keyboard::Key::RShift;
    if (str == "RAlt") return sf::Keyboard::Key::RAlt;
    if (str == "RSystem") return sf::Keyboard::Key::RSystem;
    if (str == "Menu") return sf::Keyboard::Key::Menu;
    if (str == "LBracket") return sf::Keyboard::Key::LBracket;
    if (str == "RBracket") return sf::Keyboard::Key::RBracket;
    if (str == "Semicolon") return sf::Keyboard::Key::Semicolon;
    if (str == "Comma") return sf::Keyboard::Key::Comma;
    if (str == "Period") return sf::Keyboard::Key::Period;
    if (str == "Quote") return sf::Keyboard::Key::Apostrophe; // SFML 3: Quote -> Apostrophe
    if (str == "Slash") return sf::Keyboard::Key::Slash;
    if (str == "Backslash") return sf::Keyboard::Key::Backslash;
    if (str == "Tilde") return sf::Keyboard::Key::Grave; // SFML 3: Tilde -> Grave
    if (str == "Equal") return sf::Keyboard::Key::Equal;
    if (str == "Hyphen") return sf::Keyboard::Key::Hyphen;
    if (str == "Space") return sf::Keyboard::Key::Space;
    if (str == "Enter") return sf::Keyboard::Key::Enter;
    if (str == "Backspace") return sf::Keyboard::Key::Backspace;
    if (str == "Tab") return sf::Keyboard::Key::Tab;
    if (str == "PageUp") return sf::Keyboard::Key::PageUp;
    if (str == "PageDown") return sf::Keyboard::Key::PageDown;
    if (str == "End") return sf::Keyboard::Key::End;
    if (str == "Home") return sf::Keyboard::Key::Home;
    if (str == "Insert") return sf::Keyboard::Key::Insert;
    if (str == "Delete") return sf::Keyboard::Key::Delete;
    if (str == "Add") return sf::Keyboard::Key::Add;
    if (str == "Subtract") return sf::Keyboard::Key::Subtract;
    if (str == "Multiply") return sf::Keyboard::Key::Multiply;
    if (str == "Divide") return sf::Keyboard::Key::Divide;
    if (str == "Left" || str == "LeftArrow") return sf::Keyboard::Key::Left;
    if (str == "Right" || str == "RightArrow") return sf::Keyboard::Key::Right;
    if (str == "Up" || str == "UpArrow") return sf::Keyboard::Key::Up;
    if (str == "Down" || str == "DownArrow") return sf::Keyboard::Key::Down;
    if (str == "Numpad0") return sf::Keyboard::Key::Numpad0;
    if (str == "Numpad1") return sf::Keyboard::Key::Numpad1;
    if (str == "Numpad2") return sf::Keyboard::Key::Numpad2;
    if (str == "Numpad3") return sf::Keyboard::Key::Numpad3;
    if (str == "Numpad4") return sf::Keyboard::Key::Numpad4;
    if (str == "Numpad5") return sf::Keyboard::Key::Numpad5;
    if (str == "Numpad6") return sf::Keyboard::Key::Numpad6;
    if (str == "Numpad7") return sf::Keyboard::Key::Numpad7;
    if (str == "Numpad8") return sf::Keyboard::Key::Numpad8;
    if (str == "Numpad9") return sf::Keyboard::Key::Numpad9;
    if (str == "F1") return sf::Keyboard::Key::F1;
    if (str == "F2") return sf::Keyboard::Key::F2;
    if (str == "F3") return sf::Keyboard::Key::F3;
    if (str == "F4") return sf::Keyboard::Key::F4;
    if (str == "F5") return sf::Keyboard::Key::F5;
    if (str == "F6") return sf::Keyboard::Key::F6;
    if (str == "F7") return sf::Keyboard::Key::F7;
    if (str == "F8") return sf::Keyboard::Key::F8;
    if (str == "F9") return sf::Keyboard::Key::F9;
    if (str == "F10") return sf::Keyboard::Key::F10;
    if (str == "F11") return sf::Keyboard::Key::F11;
    if (str == "F12") return sf::Keyboard::Key::F12;
    if (str == "F13") return sf::Keyboard::Key::F13;
    if (str == "F14") return sf::Keyboard::Key::F14;
    if (str == "F15") return sf::Keyboard::Key::F15;
    if (str == "Pause") return sf::Keyboard::Key::Pause;

    return sf::Keyboard::Key::Unknown;
}

std::string btnToStr(sf::Mouse::Button button)
{
    switch(button)
    {
        case sf::Mouse::Button::Left:   return "M Left";
        case sf::Mouse::Button::Right:  return "M Right";
        case sf::Mouse::Button::Middle: return "M Middle";
        case sf::Mouse::Button::Extra1: return "M X1"; // SFML 3: XButton1 -> Extra1
        case sf::Mouse::Button::Extra2: return "M X2"; // SFML 3: XButton2 -> Extra2
        default: return "M Left";
    }
}

sf::Mouse::Button strToBtn(const std::string &str)
{
    if (str == "M Left")     return sf::Mouse::Button::Left;
    if (str == "M Right")    return sf::Mouse::Button::Right;
    if (str == "M Middle")   return sf::Mouse::Button::Middle;
    if (str == "M XButton1") return sf::Mouse::Button::Extra1;
    if (str == "M XButton2") return sf::Mouse::Button::Extra2;

    return sf::Mouse::Button::Left;
}

bool isKey(const std::string &str)
{
    sf::Keyboard::Key key = strToKey(str);
    if (key != sf::Keyboard::Key::Unknown)
        return true;
    // Also check scancode-only keys (e.g. NonUsBackslash)
    sf::Keyboard::Scancode sc = strToScancode(str);
    return sc != sf::Keyboard::Scancode::Unknown;
}

bool isButton(const std::string &str)
{
    sf::Mouse::Button btn = strToBtn(str);
    // strToBtn returns sf::Mouse::Button::Left if it couldn't find right value
    return btn == sf::Mouse::Button::Left ? str == "M Left" : true;
}

std::string scancodeToStr(sf::Keyboard::Scancode scancode, bool saveToCfg)
{
    // Handle punctuation/symbol scancodes directly to avoid localize() failures on
    // non-US keyboard layouts (e.g. German) where these keys produce characters with
    // no sf::Keyboard::Key enum value, causing localize() to return Unknown.
    switch (scancode)
    {
        case sf::Keyboard::Scancode::NonUsBackslash: return saveToCfg ? "NonUsBackslash" : "<>";
        case sf::Keyboard::Scancode::LBracket:   return saveToCfg ? "LBracket"  : "[";
        case sf::Keyboard::Scancode::RBracket:   return saveToCfg ? "RBracket"  : "]";
        case sf::Keyboard::Scancode::Semicolon:  return saveToCfg ? "Semicolon" : ";";
        case sf::Keyboard::Scancode::Apostrophe: return saveToCfg ? "Quote"     : "'";
        case sf::Keyboard::Scancode::Grave:      return saveToCfg ? "Tilde"     : "`";
        case sf::Keyboard::Scancode::Backslash:  return saveToCfg ? "Backslash" : "\\";
        case sf::Keyboard::Scancode::Equal:      return saveToCfg ? "Equal"     : "=";
        case sf::Keyboard::Scancode::Hyphen:     return saveToCfg ? "Hyphen"    : "-";
        case sf::Keyboard::Scancode::Comma:      return saveToCfg ? "Comma"     : ",";
        case sf::Keyboard::Scancode::Period:     return saveToCfg ? "Period"    : ".";
        case sf::Keyboard::Scancode::Slash:      return saveToCfg ? "Slash"     : "/";
        default: break;
    }

    // For all other scancodes, convert to Key and use existing keyToStr
    sf::Keyboard::Key key = sf::Keyboard::localize(scancode);
    if (key != sf::Keyboard::Key::Unknown)
        return keyToStr(key, saveToCfg);

    // Fallback: return the scancode description from SFML
    return sf::Keyboard::getDescription(scancode).toAnsiString();
}

sf::Keyboard::Scancode strToScancode(const std::string &str)
{
    // Handle punctuation/symbol keys directly to avoid delocalize() failures on
    // non-US keyboard layouts where these keys may not exist as sf::Keyboard::Key values.
    if (str == "NonUsBackslash" || str == "<>" || str == "<" || str == ">")
        return sf::Keyboard::Scancode::NonUsBackslash;
    if (str == "LBracket"  || str == "[")  return sf::Keyboard::Scancode::LBracket;
    if (str == "RBracket"  || str == "]")  return sf::Keyboard::Scancode::RBracket;
    if (str == "Semicolon" || str == ";")  return sf::Keyboard::Scancode::Semicolon;
    if (str == "Quote"     || str == "'")  return sf::Keyboard::Scancode::Apostrophe;
    if (str == "Tilde"     || str == "`")  return sf::Keyboard::Scancode::Grave;
    if (str == "Backslash" || str == "\\") return sf::Keyboard::Scancode::Backslash;
    if (str == "Equal"     || str == "=")  return sf::Keyboard::Scancode::Equal;
    if (str == "Hyphen"    || str == "-")  return sf::Keyboard::Scancode::Hyphen;
    if (str == "Comma"     || str == ",")  return sf::Keyboard::Scancode::Comma;
    if (str == "Period"    || str == ".")  return sf::Keyboard::Scancode::Period;
    if (str == "Slash"     || str == "/")  return sf::Keyboard::Scancode::Slash;

    // For all other strings, convert via Key and delocalize
    sf::Keyboard::Key key = strToKey(str);
    if (key != sf::Keyboard::Key::Unknown)
        return sf::Keyboard::delocalize(key);

    return sf::Keyboard::Scancode::Unknown;
}

std::string logKeyToStr(const LogKey &logKey)
{
    if (logKey.keyboardKey)
        return scancodeToStr(*logKey.keyboardKey);
    else
        return btnToStr(*logKey.mouseButton);
}

char enumKeyToStr(sf::Keyboard::Key key)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
    ||  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift))
    {
        switch (key)
        {
            case sf::Keyboard::Key::A: return 'A';
            case sf::Keyboard::Key::B: return 'B';
            case sf::Keyboard::Key::C: return 'C';
            case sf::Keyboard::Key::D: return 'D';
            case sf::Keyboard::Key::E: return 'E';
            case sf::Keyboard::Key::F: return 'F';
            case sf::Keyboard::Key::G: return 'G';
            case sf::Keyboard::Key::H: return 'H';
            case sf::Keyboard::Key::I: return 'I';
            case sf::Keyboard::Key::J: return 'J';
            case sf::Keyboard::Key::K: return 'K';
            case sf::Keyboard::Key::L: return 'L';
            case sf::Keyboard::Key::M: return 'M';
            case sf::Keyboard::Key::N: return 'N';
            case sf::Keyboard::Key::O: return 'O';
            case sf::Keyboard::Key::P: return 'P';
            case sf::Keyboard::Key::Q: return 'Q';
            case sf::Keyboard::Key::R: return 'R';
            case sf::Keyboard::Key::S: return 'S';
            case sf::Keyboard::Key::T: return 'T';
            case sf::Keyboard::Key::U: return 'U';
            case sf::Keyboard::Key::V: return 'V';
            case sf::Keyboard::Key::W: return 'W';
            case sf::Keyboard::Key::X: return 'X';
            case sf::Keyboard::Key::Y: return 'Y';
            case sf::Keyboard::Key::Z: return 'Z';
            case sf::Keyboard::Key::Numpad0:
            case sf::Keyboard::Key::Num0: return ')';
            case sf::Keyboard::Key::Numpad1:
            case sf::Keyboard::Key::Num1: return '!';
            case sf::Keyboard::Key::Numpad2:
            case sf::Keyboard::Key::Num2: return '@';
            case sf::Keyboard::Key::Numpad3:
            case sf::Keyboard::Key::Num3: return '#';
            case sf::Keyboard::Key::Numpad4:
            case sf::Keyboard::Key::Num4: return '$';
            case sf::Keyboard::Key::Numpad5:
            case sf::Keyboard::Key::Num5: return '%';
            case sf::Keyboard::Key::Numpad6:
            case sf::Keyboard::Key::Num6: return '^';
            case sf::Keyboard::Key::Numpad7:
            case sf::Keyboard::Key::Num7: return '&';
            case sf::Keyboard::Key::Numpad8:
            case sf::Keyboard::Key::Num8: return '*';
            case sf::Keyboard::Key::Numpad9:
            case sf::Keyboard::Key::Num9: return '(';
            case sf::Keyboard::Key::LBracket: return '{';
            case sf::Keyboard::Key::RBracket: return '}';
            case sf::Keyboard::Key::Semicolon: return ':';
            case sf::Keyboard::Key::Comma: return '<';
            case sf::Keyboard::Key::Period: return '>';
            case sf::Keyboard::Key::Apostrophe: return '"';
            case sf::Keyboard::Key::Slash: return '?';
            case sf::Keyboard::Key::Backslash: return '|';
            case sf::Keyboard::Key::Grave: return '~';
            case sf::Keyboard::Key::Equal: return '+';
            case sf::Keyboard::Key::Hyphen: return '_';
            case sf::Keyboard::Key::Space: return ' ';
            case sf::Keyboard::Key::Add: return '+';
            case sf::Keyboard::Key::Subtract: return '-';
            case sf::Keyboard::Key::Multiply: return '*';
            case sf::Keyboard::Key::Divide: return '/';
            default: return ' ';
        }
    }
    else
    {
        switch (key)
        {
            case sf::Keyboard::Key::A: return 'a';
            case sf::Keyboard::Key::B: return 'b';
            case sf::Keyboard::Key::C: return 'c';
            case sf::Keyboard::Key::D: return 'd';
            case sf::Keyboard::Key::E: return 'e';
            case sf::Keyboard::Key::F: return 'f';
            case sf::Keyboard::Key::G: return 'g';
            case sf::Keyboard::Key::H: return 'h';
            case sf::Keyboard::Key::I: return 'i';
            case sf::Keyboard::Key::J: return 'j';
            case sf::Keyboard::Key::K: return 'k';
            case sf::Keyboard::Key::L: return 'l';
            case sf::Keyboard::Key::M: return 'm';
            case sf::Keyboard::Key::N: return 'n';
            case sf::Keyboard::Key::O: return 'o';
            case sf::Keyboard::Key::P: return 'p';
            case sf::Keyboard::Key::Q: return 'q';
            case sf::Keyboard::Key::R: return 'r';
            case sf::Keyboard::Key::S: return 's';
            case sf::Keyboard::Key::T: return 't';
            case sf::Keyboard::Key::U: return 'u';
            case sf::Keyboard::Key::V: return 'v';
            case sf::Keyboard::Key::W: return 'w';
            case sf::Keyboard::Key::X: return 'x';
            case sf::Keyboard::Key::Y: return 'y';
            case sf::Keyboard::Key::Z: return 'z';
            case sf::Keyboard::Key::Numpad0:
            case sf::Keyboard::Key::Num0: return '0';
            case sf::Keyboard::Key::Numpad1:
            case sf::Keyboard::Key::Num1: return '1';
            case sf::Keyboard::Key::Numpad2:
            case sf::Keyboard::Key::Num2: return '2';
            case sf::Keyboard::Key::Numpad3:
            case sf::Keyboard::Key::Num3: return '3';
            case sf::Keyboard::Key::Numpad4:
            case sf::Keyboard::Key::Num4: return '4';
            case sf::Keyboard::Key::Numpad5:
            case sf::Keyboard::Key::Num5: return '5';
            case sf::Keyboard::Key::Numpad6:
            case sf::Keyboard::Key::Num6: return '6';
            case sf::Keyboard::Key::Numpad7:
            case sf::Keyboard::Key::Num7: return '7';
            case sf::Keyboard::Key::Numpad8:
            case sf::Keyboard::Key::Num8: return '8';
            case sf::Keyboard::Key::Numpad9:
            case sf::Keyboard::Key::Num9: return '9';
            case sf::Keyboard::Key::LBracket: return '[';
            case sf::Keyboard::Key::RBracket: return ']';
            case sf::Keyboard::Key::Semicolon: return ';';
            case sf::Keyboard::Key::Comma: return ',';
            case sf::Keyboard::Key::Period: return '.';
            case sf::Keyboard::Key::Apostrophe: return '\'';
            case sf::Keyboard::Key::Slash: return '/';
            case sf::Keyboard::Key::Backslash: return '\\';
            case sf::Keyboard::Key::Grave: return '`';
            case sf::Keyboard::Key::Equal: return '=';
            case sf::Keyboard::Key::Hyphen: return '-';
            case sf::Keyboard::Key::Space: return ' ';
            case sf::Keyboard::Key::Add: return '+';
            case sf::Keyboard::Key::Subtract: return '-';
            case sf::Keyboard::Key::Multiply: return '*';
            case sf::Keyboard::Key::Divide: return '/';
            default: return ' ';
        }
    }
}