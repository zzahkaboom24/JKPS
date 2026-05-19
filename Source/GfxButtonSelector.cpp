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
    if (!mFont.openFromMemory(RobotoMono, 1100000))
        throw std::runtime_error("KeySelector::KeySelector - Failed to load default font");

    auto realKeyGfx = std::make_unique<GfxParameter>(&mFont, "Key", 0, sf::Vector2f({150.f, 25.f}));
    realKeyGfx->setPosition({mWindowSize.x / 2.f, 25.f});
    mButtons[RealKeyButton] = std::move(realKeyGfx);
    
    auto visualKeyGfx = std::make_unique<GfxParameter>(&mFont, "Visual key", 0, sf::Vector2f({250.f, 25.f}));
    visualKeyGfx->setPosition({mWindowSize.x / 2.f, 75.f});
    // Make the text gray in order to show that it is a hint, not an actual text
    visualKeyGfx->mValText->setFillColor(mDefaultVisualKeyColor);
    mButtons[VisualKeyButton] = std::move(visualKeyGfx);

    auto acceptButton = std::make_unique<GfxParameter>(nullptr, true);
    acceptButton->setPosition({mWindowSize.x / 2.f, 125.f});
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

        auto str = static_cast<std::string>(mSelectedBtn->mValText->getString());

        if (mSelectedBtn == mButtons[RealKeyButton].get())
        {
            const auto visualKeyChanged = mButtons[VisualKeyButton]->mValText->getString() != 
                scancodeToStr(strToScancode(std::string(mButtons[RealKeyButton]->mValText->getString())));

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
                rmChOnIdx(str, mSelectedBtnTextIndex - 1);
                --mSelectedBtnTextIndex;
            }
            strChanged = true;
        }
        else if (kp->code == sf::Keyboard::Key::Delete)
        {
            if (str.size() > static_cast<size_t>(mSelectedBtnTextIndex))
                rmChOnIdx(str, mSelectedBtnTextIndex);
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
     
        if (isCharacter(kp->code))
        {
            const auto maxLength = 20ul;
            if (mButtons[VisualKeyButton]->mValText->getString().getSize() >= maxLength)
                return;

            if (kp->control && kp->code == sf::Keyboard::Key::V)
            {
                const auto clipboardStr = std::string(sf::Clipboard::getString());
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
            else
            {
                addChOnIdx(str, mSelectedBtnTextIndex, enumKeyToStr(kp->code));
                ++mSelectedBtnTextIndex;
                strChanged = true;
            }
        }

        // If user starts to write anything, then change text color and delete the hint
        if (mButtons[VisualKeyButton]->mValText->getFillColor() == mDefaultVisualKeyColor && strChanged)
        {
            mButtons[VisualKeyButton]->mValText->setFillColor(sf::Color::White);
            mSelectedBtn->mValText->setString("");
            str = "";
            if (isCharacter(kp->code))
                str += enumKeyToStr(kp->code);
            mSelectedBtnTextIndex = static_cast<int>(str.size());
        }
        mSelectedBtn->mValText->setString(str);
        mSelectedBtn->setupValPos();
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
    mButtons[VisualKeyButton]->mValText->setString(mLogKey->visualStr);
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
#elif linux
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
    mLogKey->realStr = mButtons[RealKeyButton]->mValText->getString();
    mLogKey->visualStr = mButtons[VisualKeyButton]->mValText->getString();

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
    auto x = static_cast<float>(mSelectedBtn->getPosition().x - mSelectedBtn->mRect.getSize().x / 2.f + 
        (mSelectedBtn->mRect.getSize().x - mSelectedBtn->mValText->getLocalBounds().size.x) / 2.f +
        mSelectedBtnTextIndex * (chSz.x - textOpt->getLetterSpacing() * 2.f));
    auto y = static_cast<float>(mCursor.getPosition().y);

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
                strToStr = scancodeToStr(strToScancode(std::string(mButtons[RealKeyButton]->mValText->getString()))); 
                break;

            case Mouse: 
                strToStr = mButtons[RealKeyButton]->mValText->getString(); 
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
