#include "../Headers/GfxButtonSelector.hpp"
#include "../Headers/Default media/Fonts/RobotoMono.hpp"
#include "../Headers/StringHelper.hpp"
#include "../Headers/Settings.hpp"

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Event.hpp>

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <cstdint>

sf::RectangleShape GfxButtonSelector::mCursor(sf::Vector2f({1.f, 21.f}));
int GfxButtonSelector::mSelectedBtnTextIndex(-1);
GfxParameter *GfxButtonSelector::mSelectedBtn(nullptr);
const std::string GfxButtonSelector::mDefaultVisualKeyStr("Visual key");
const sf::Color GfxButtonSelector::mDefaultVisualKeyColor(sf::Color(160, 160, 160));

GfxButtonSelector::GfxButtonSelector()
: mWindowSize(300u, 200u)
, mLogKey(nullptr)
, mKeyType(Keyboard)
{
    if (!mFont.openFromMemory(RobotoMono, RobotoMono_size))
        throw std::runtime_error("KeySelector::KeySelector - Failed to load default font");

    auto realKeyGfx = std::make_unique<GfxParameter>(&mFont, "Key", 0, sf::Vector2f({150.f, 25.f}));
    realKeyGfx->setPosition({static_cast<float>(mWindowSize.x) / 2.f, 25.f});
    mButtons[RealKeyButton] = std::move(realKeyGfx);
    
    auto visualKeyGfx = std::make_unique<GfxParameter>(&mFont, "Visual key", 0, sf::Vector2f({250.f, 25.f}));
    visualKeyGfx->setPosition({static_cast<float>(mWindowSize.x) / 2.f, 75.f});
    // Make the text gray in order to show that it is a hint, not an actual text
    visualKeyGfx->mValText->setFillColor(mDefaultVisualKeyColor);
    mButtons[VisualKeyButton] = std::move(visualKeyGfx);

    auto acceptButton = std::make_unique<GfxParameter>(nullptr, true);
    acceptButton->setPosition({static_cast<float>(mWindowSize.x) / 2.f, 125.f});
    mButtons[AcceptButton] = std::move(acceptButton);

    mCursor.setOutlineThickness(1.f);
    mCursor.setFillColor(sf::Color::White);
    mCursor.setOutlineColor(sf::Color::Black);
    mCursor.setOrigin(mCursor.getSize() / 2.f);
    // set visual key button height since it is the only one gfx button where the cursor is needed
    mCursor.setPosition({0.f, mButtons[VisualKeyButton]->getPosition().y}); 
}

void GfxButtonSelector::handleOwnInput()
{
    while (const std::optional<sf::Event> optEvent = mWindow.pollEvent())
    {
        const sf::Event &event = *optEvent;
        auto handleExit = [this] ()
            {
                deselect();
                mWindow.close();
            };
        if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
        {
            if (Settings::KeyExit.isTriggered(kp))
            {
                handleExit();
            }
        }

        if (event.is<sf::Event::Closed>())
            handleExit();

        handleButtonModificationEvent(event);
        handleButtonInteractionEvent(event);
    }
}

void GfxButtonSelector::handleButtonModificationEvent(const sf::Event& event)
{
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {    
        if (!mSelectedBtn || (mSelectedBtn == mButtons[RealKeyButton].get() 
        &&  mKeyType == Mouse))
            return;

        auto str = mSelectedBtn->mValText->getString().toAnsiString();

        if (mSelectedBtn == mButtons[RealKeyButton].get())
        {
            const auto visualKeyChanged = mButtons[VisualKeyButton]->mValText->getString() !=
                scancodeToStr(strToScancode(mButtons[RealKeyButton]->mValText->getString().toAnsiString()));

            mButtons[RealKeyButton]->mValText->setString(kp->scancode != sf::Keyboard::Scancode::Unknown ? scancodeToStr(kp->scancode, true) : "Unknown");
            mButtons[RealKeyButton]->setupValPos();

            if (!visualKeyChanged)
            {
                mButtons[VisualKeyButton]->mValText->setString(kp->scancode != sf::Keyboard::Scancode::Unknown ? scancodeToStr(kp->scancode) : "Unknown");
                mButtons[VisualKeyButton]->setupValPos();
            }
            deselect();
            return;
        }

        auto strChanged = false;
        if (kp->code == sf::Keyboard::Key::Backspace)
        {
            if (mSelectedBtnTextIndex != 0)
            {
                rmChOnIdx(str, static_cast<unsigned>(mSelectedBtnTextIndex - 1));
                --mSelectedBtnTextIndex;
            }
            strChanged = true;
        }
        else if (kp->code == sf::Keyboard::Key::Delete)
        {
            if (str.size() > static_cast<size_t>(mSelectedBtnTextIndex))
                rmChOnIdx(str, static_cast<unsigned>(mSelectedBtnTextIndex));
            strChanged = true;
        }
        else if (kp->code == sf::Keyboard::Key::Left)
        {
            if (mSelectedBtnTextIndex > 0)
                --mSelectedBtnTextIndex;
        }
        else if (kp->code == sf::Keyboard::Key::Right)
        {
            if (str.size() > static_cast<size_t>(mSelectedBtnTextIndex))
                ++mSelectedBtnTextIndex;
        }
		else if (kp->code == sf::Keyboard::Key::Home)
		{
			mSelectedBtnTextIndex = 0;
		}
        else if (kp->code == sf::Keyboard::Key::End)
        {
            mSelectedBtnTextIndex = static_cast<int>(str.size());
        }
     
        if (kp->control && kp->code == sf::Keyboard::Key::V)
        {
            const auto maxLength = 20ul;
            if (mButtons[VisualKeyButton]->mValText->getString().getSize() < maxLength)
            {
                const auto clipboardStr = sf::Clipboard::getString().toAnsiString();
                const auto lhs = std::string(str.begin(), str.begin() + mSelectedBtnTextIndex);
                const auto rhs = std::string(str.begin() + mSelectedBtnTextIndex, str.end());
                const auto newStr = lhs + clipboardStr + rhs;

                if (maxLength >= newStr.length())
                {
                    str = newStr;
                    mSelectedBtnTextIndex += static_cast<int>(clipboardStr.length());
                    strChanged = true;
                }
            }
        }

        if (strChanged)
        {
            // If user starts to write anything, change text color and delete the hint
            if (mButtons[VisualKeyButton]->mValText->getFillColor() == mDefaultVisualKeyColor)
            {
                mButtons[VisualKeyButton]->mValText->setFillColor(sf::Color::White);
                mSelectedBtn->mValText->setString("");
                str = "";
                mSelectedBtnTextIndex = 0;
            }
            mSelectedBtn->mValText->setString(str);
            mSelectedBtn->setupValPos();
            setCursorPos();
        }
    }

    if (const auto* te = event.getIf<sf::Event::TextEntered>())
    {
        if (!mSelectedBtn || mSelectedBtn != mButtons[VisualKeyButton].get())
            return;

        const char32_t unicode = te->unicode;
        // Skip control characters; accept all printable Unicode
        if (unicode < 32 || unicode == 127)
            return;

        const auto maxLength = 20ul;
        if (mButtons[VisualKeyButton]->mValText->getString().getSize() >= maxLength)
            return;

        // Convert to UTF-8 and append; for ASCII range this is a single byte
        sf::String current = mButtons[VisualKeyButton]->mValText->getString();
        // Insert at cursor position
        sf::String lhs = current.substring(0, static_cast<std::size_t>(mSelectedBtnTextIndex));
        sf::String rhs = (static_cast<std::size_t>(mSelectedBtnTextIndex) < current.getSize())
            ? current.substring(static_cast<std::size_t>(mSelectedBtnTextIndex)) : sf::String();
        sf::String newStr = lhs + sf::String(unicode) + rhs;

        // If hint color is still set, clear the hint first
        if (mButtons[VisualKeyButton]->mValText->getFillColor() == mDefaultVisualKeyColor)
        {
            mButtons[VisualKeyButton]->mValText->setFillColor(sf::Color::White);
            newStr = sf::String(unicode);
            mSelectedBtnTextIndex = 0;
        }

        mButtons[VisualKeyButton]->mValText->setString(newStr);
        ++mSelectedBtnTextIndex;
        mButtons[VisualKeyButton]->setupValPos();
        setCursorPos();
    }

    if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>())
    {
        const auto button = mbp->button;
        if (mSelectedBtn == mButtons[RealKeyButton].get() && mKeyType == Mouse)
        {
            const auto visualKeyChanged = mButtons[VisualKeyButton]->mValText->getString() != mButtons[RealKeyButton]->mValText->getString();

            mButtons[RealKeyButton]->mValText->setString(btnToStr(button));
            mButtons[RealKeyButton]->setupValPos();

            if (!visualKeyChanged)
            {
                mButtons[VisualKeyButton]->mValText->setString(mButtons[RealKeyButton]->mValText->getString());
                mButtons[VisualKeyButton]->setupValPos();
            }
            deselect();
            return;
        }
    }
}

void GfxButtonSelector::handleButtonInteractionEvent(const sf::Event& event)
{
    if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>())
    {
        const auto button = mbp->button;
        for (auto &elem : mButtons)
        {
            const auto mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(mWindow));

            if (button == sf::Mouse::Button::Left && elem->contains(mousePos))
            {
                if (elem == mButtons[VisualKeyButton] 
                &&  mButtons[VisualKeyButton]->mValText->getFillColor() == mDefaultVisualKeyColor)
                {
                    mButtons[VisualKeyButton]->mValText->setString("");
                }
                if (elem == mButtons[AcceptButton])
                {
                    saveKey();
                    mWindow.close();
                    return;
                }
                deselect();
                select(elem.get());
            } 
            else if ((button == sf::Mouse::Button::Left || button == sf::Mouse::Button::Right) 
            && elem.get() == mSelectedBtn)
            {
                deselect();
            }
        }
    }
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        const auto key = kp->code;
        if (key == sf::Keyboard::Key::Escape)
        {
            deselect();
        }
        if (mSelectedBtn != mButtons[RealKeyButton].get() && key == sf::Keyboard::Key::Enter)
        {
            saveKey();
            mWindow.close();
            return;
        }
    }
}

void GfxButtonSelector::render()
{
    mWindow.clear(sf::Color(45,45,45));

    for (const auto &elem : mButtons)
        mWindow.draw(*elem);
    if (mSelectedBtnTextIndex != -1 && mSelectedBtn != mButtons[RealKeyButton].get()
    &&  mButtons[VisualKeyButton]->mValText->getFillColor() != mDefaultVisualKeyColor)
        mWindow.draw(mCursor);

    mWindow.display();
}

void GfxButtonSelector::setKey(LogKey *logKey)
{
    assert(logKey);

    mLogKey = logKey;
    mKeyType = mLogKey->keyboardKey ? Keyboard : Mouse;

    mButtons[RealKeyButton]->mValText->setString(mLogKey->realStr);
    mButtons[VisualKeyButton]->mValText->setString(sf::String::fromUtf8(mLogKey->visualStr.begin(), mLogKey->visualStr.end()));
    resetVisualKeyGfxButton(mLogKey->realStr, mLogKey->visualStr);

    mButtons[RealKeyButton]->setupValPos();
    mButtons[VisualKeyButton]->setupValPos();
    select(mButtons[RealKeyButton].get());
}

void GfxButtonSelector::openWindow()
{
    if (!mWindow.isOpen())
    {
        const auto title = std::string(mKeyType == Keyboard ? "Keyboard key selector" : "Mouse button selector");

        std::uint32_t style;
#ifdef _WIN32
        style = sf::Style::Close;
#elif __linux__
        style = sf::Style::Default;
#else
#error Unsupported compiler
#endif

        mWindow.create(sf::VideoMode({300u, 150u}), title, style);
        mWindow.requestFocus();
    }
}

bool GfxButtonSelector::isOpen()
{
    return mWindow.isOpen();
}

void GfxButtonSelector::select(GfxParameter *ptr)
{
    mSelectedBtn = ptr;
    // Make the text gray in order to show that it is a hint, not an actual text
    mSelectedBtn->mRect.setFillColor(GfxParameter::defaultSelectedRectColor);
    mSelectedBtnTextIndex = static_cast<int>(mSelectedBtn->mValText->getString().getSize());
    setCursorPos();
}

void GfxButtonSelector::deselect()
{
    if (mSelectedBtn == nullptr)
        return;

    if (mSelectedBtn == mButtons[VisualKeyButton].get() 
    && mButtons[VisualKeyButton]->mValText->getString() == "")
    {
        resetVisualKeyGfxButton("", "");
    }

    mSelectedBtn->mRect.setFillColor(GfxParameter::defaultRectColor);
    mSelectedBtn = nullptr;
    mSelectedBtnTextIndex = -1;
}

void GfxButtonSelector::saveKey()
{
    mLogKey->realStr = mButtons[RealKeyButton]->mValText->getString().toAnsiString();
    const auto utf8 = mButtons[VisualKeyButton]->mValText->getString().toUtf8();
    mLogKey->visualStr = std::string(utf8.begin(), utf8.end());

    switch(mKeyType)
    {
        case Keyboard:
            *mLogKey->keyboardKey = strToScancode(mLogKey->realStr);
            break;
        case Mouse:
            *mLogKey->mouseButton = strToBtn(mLogKey->realStr);
            break;
    }

    mLogKey->changed = true;
    mLogKey = nullptr;
    deselect();
}

void GfxButtonSelector::setCursorPos()
{
    if (!mSelectedBtn)
        return;

    static std::optional<sf::Text> textOpt;
    static auto chSz = sf::Vector2f();
    if (!textOpt)
    {
        textOpt.emplace(mFont);
        textOpt->setString("0");
        chSz.x = textOpt->getLocalBounds().size.x;
        chSz.y = textOpt->getLocalBounds().size.y;
    }

    // Take absolute position of the center of the button, substract by half width - the cursor is on the left bound,
    // then find the width of the part on the text left, and add it - the cursor is on the text left,
    // then take space in X axes for each character, substract it by 2 times spacing between them, 
    // and multiply by current cursor index - the cursor is on the index left
    auto x = mSelectedBtn->getPosition().x - mSelectedBtn->mRect.getSize().x / 2.f +
        (mSelectedBtn->mRect.getSize().x - mSelectedBtn->mValText->getLocalBounds().size.x) / 2.f +
        static_cast<float>(mSelectedBtnTextIndex) * (chSz.x - textOpt->getLetterSpacing() * 2.f);
    auto y = mCursor.getPosition().y;

    mCursor.setPosition({x, y});
}

void GfxButtonSelector::resetVisualKeyGfxButton(const std::string &str1, const std::string &str2)
{
    if (str1 == str2)
    {
        std::string strToStr;
        switch(mKeyType)
        {
            case Keyboard:
                strToStr = scancodeToStr(strToScancode(mButtons[RealKeyButton]->mValText->getString().toAnsiString()));
                break;

            case Mouse:
                strToStr = mButtons[RealKeyButton]->mValText->getString().toAnsiString();
                break;
        }
            
        mButtons[VisualKeyButton]->mValText->setString(strToStr);
    }
    mButtons[VisualKeyButton]->mValText->setFillColor(mDefaultVisualKeyColor);
    mButtons[VisualKeyButton]->setupValPos();
}

bool GfxButtonSelector::isCharacter(sf::Keyboard::Key key)
{
    return 
        (key >= sf::Keyboard::Key::A         && key <= sf::Keyboard::Key::Num9)
    ||  (key >= sf::Keyboard::Key::LBracket  && key <= sf::Keyboard::Key::Space)
    ||  (key >= sf::Keyboard::Key::Add       && key <= sf::Keyboard::Key::Divide)
    ||  (key >= sf::Keyboard::Key::Numpad0   && key <= sf::Keyboard::Key::Numpad9);
}
