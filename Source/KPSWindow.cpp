#include "../Headers/KPSWindow.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/Button.hpp"
#include "../Headers/StringHelper.hpp"

#include <SFML/Window/Event.hpp>

#include <optional>
#include <cstdint>

KPSWindow::KPSWindow(const FontHolder& fonts)
: mFonts(fonts)
, mKPSText(fonts.get(Fonts::KPSText))
, mKPSNumber(fonts.get(Fonts::KPSNumber))
{
    updateAssets();
    updateParameters();

    mKPSText.setString("KPS");
    mKPSNumber.setString("0");

    if (Settings::KPSWindowEnabledFromStart)
        openWindow();
}

void KPSWindow::handleOwnEvent()
{
    while (const std::optional event = mWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            mWindow.close();
        }

        if (const auto* keyPressed =
                event->getIf<sf::Event::KeyPressed>())
        {
            if (Settings::KeyExit.isTriggered(keyPressed))
            {
                mWindow.close();
            }
        }
    }
}

void KPSWindow::update()
{
    if (mWindow.isOpen())
    {
        std::string str;

        const auto kps = Button::getKeysPerSecond();

        if (false)
            str = eraseDigitsOverHundredths(std::to_string(kps));
        else
            str = std::to_string(static_cast<unsigned>(kps));

        mKPSNumber.setString(str);

        const auto bounds = mKPSNumber.getLocalBounds();

        mKPSNumber.setOrigin({
            (bounds.position.x + bounds.size.x) / 2.f,
            bounds.position.y
        });
    }
}

void KPSWindow::render()
{
    if (mWindow.isOpen())
    {
        auto textTransform = sf::Transform::Identity;
        auto numberTransform = sf::Transform::Identity;

        const auto textBounds = mKPSText.getLocalBounds();
        const auto numberBounds = mKPSNumber.getLocalBounds();

        textTransform.translate({
            (static_cast<float>(mWindow.getSize().x)
                - textBounds.size.x
                + textBounds.position.x) / 2.f,
            Settings::KPSWindowTopPadding
        });

        numberTransform.translate({
            static_cast<float>(mWindow.getSize().x) / 2.f
                - numberBounds.position.x,

            textBounds.size.y
                + Settings::KPSWindowDistanceBetween
                + Settings::KPSWindowTopPadding
        });

        mWindow.clear(Settings::KPSBackgroundColor);

        mWindow.draw(mKPSText, textTransform);
        mWindow.draw(mKPSNumber, numberTransform);

        mWindow.display();
    }
}

void KPSWindow::updateParameters()
{
    {
        const auto bounds = mKPSText.getLocalBounds();

        mKPSText.setOrigin({
            bounds.position.x,
            bounds.position.y
        });
    }

    {
        const auto bounds = mKPSNumber.getLocalBounds();

        mKPSNumber.setOrigin({
            (bounds.position.x + bounds.size.x) / 2.f,
            bounds.position.y
        });
    }

    mKPSText.setCharacterSize(Settings::KPSTextSize);
    mKPSNumber.setCharacterSize(Settings::KPSNumberSize);

    mKPSText.setFillColor(Settings::KPSTextColor);
    mKPSNumber.setFillColor(Settings::KPSNumberColor);

    mWindow.setSize(sf::Vector2u(
        Settings::KPSWindowSize.x,
        Settings::KPSWindowSize.y));

    mWindow.setView(sf::View(sf::FloatRect(
        {0.f, 0.f},
        {
            static_cast<float>(Settings::KPSWindowSize.x),
            static_cast<float>(Settings::KPSWindowSize.y)
        }
    )));
}

void KPSWindow::updateAssets()
{
    mKPSText.setFont(mFonts.get(Fonts::KPSText));
    mKPSNumber.setFont(mFonts.get(Fonts::KPSNumber));
}

void KPSWindow::openWindow()
{
    std::uint32_t style;

#ifdef _WIN32
    style = sf::Style::Close;
#elif __linux__
    style = sf::Style::Default;
#else
#error Unsupported compiler
#endif

    mWindow.create(
        sf::VideoMode({
            Settings::KPSWindowSize.x,
            Settings::KPSWindowSize.y
        }),
        "KPS Window",
        style);

#ifdef linux
    auto desktop = sf::VideoMode::getDesktopMode();

    mWindow.setPosition(sf::Vector2i(
        static_cast<int>(desktop.size.x / 1.5f
            - mWindow.getSize().x / 2.f),

        static_cast<int>(desktop.size.y / 2.f
            - mWindow.getSize().y / 2.f)
    ));
#endif
}

void KPSWindow::closeWindow()
{
    mWindow.close();
}

bool KPSWindow::isOpen() const
{
    return mWindow.isOpen();
}

bool KPSWindow::parameterIdMatches(LogicalParameter::ID id)
{
    return
        id == LogicalParameter::ID::KPSWndwSz ||
        id == LogicalParameter::ID::KPSWndwTxtChSz ||
        id == LogicalParameter::ID::KPSWndwNumChSz ||
        id == LogicalParameter::ID::KPSWndwBgClr ||
        id == LogicalParameter::ID::KPSWndwTxtClr ||
        id == LogicalParameter::ID::KPSWndwNumClr ||
        id == LogicalParameter::ID::KPSWndwTopPadding ||
        id == LogicalParameter::ID::KPSWndwDistBtw;
}