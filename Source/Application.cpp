#include "../Headers/Application.hpp"
#include "../Headers/Menu.hpp"
#include "../Headers/LogicalParameter.hpp"
#include "../Headers/ChangedParametersQueue.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/DefaultFiles.hpp"
#include "../Headers/Menu.hpp"
#include "../Headers/ConfigHelper.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <optional>
#include <cstdint>

const unsigned HooksUpdateFrequency = 60u;
const sf::Time Application::TimePerHookUpdate = sf::seconds(1.f / static_cast<float>(HooksUpdateFrequency));

Application::Application()
{
    loadTextures();
    loadFonts();

    buildButtons();
    buildStatistics();

    openWindow();
    loadIcon();

    auto keySelector = std::make_unique<GfxButtonSelector>();
    mGfxButtonSelector = std::move(keySelector);

    auto buttonPositioner = std::make_unique<ButtonPositioner>(&mButtons);
    mButtonsPositioner = std::move(buttonPositioner);
    (*mButtonsPositioner)();

    auto statPositioner = std::make_unique<StatisticsPositioner>(&mStatistics);
    mStatisticsPositioner = std::move(statPositioner);
    (*mStatisticsPositioner)();

    auto bg = std::make_unique<Background>(mTextures, mWindow);
    mBackground = std::move(bg);

    auto kpsWindow = std::make_unique<KPSWindow>(mFonts);
    mKPSWindow = std::move(kpsWindow);

    auto graph = std::make_unique<KeysPerSecondGraph>();
    mGraph = std::move(graph);

    mMenu.saveConfig(mButtons);
}

void Application::run()
{
    sf::Clock clock;
    auto timeSinceLastEventUpdate = sf::Time::Zero;
    auto timeSinceLastHooksUpdate = sf::Time::Zero;

    while (mWindow.isOpen())
    {
        auto dt = clock.restart();
        timeSinceLastEventUpdate += dt;
        timeSinceLastHooksUpdate += dt;

        while (true)
        {
            int updateType = UpdateType::None;
            if (timeSinceLastHooksUpdate > TimePerHookUpdate)
            {
                timeSinceLastHooksUpdate -= TimePerHookUpdate;
                updateType |= UpdateType::Hooks;
            }

            const sf::Time TimePerEventUpdate = sf::seconds(1.f / static_cast<float>(getRenderUpdateFrequency()));
            if (timeSinceLastEventUpdate > TimePerEventUpdate)
            {
                timeSinceLastEventUpdate -= TimePerEventUpdate;
                updateType |= UpdateType::Event;
            }

            if (updateType == UpdateType::None)
            {
                break;
            }

            processInput(static_cast<UpdateType>(updateType));
            update(TimePerEventUpdate.asSeconds(), static_cast<UpdateType>(updateType));
        }

        render();
    }
}

void Application::processInput(UpdateType type)
{
    if (type & UpdateType::Event)
    {
        // Open/close other windows, add/rm keys
        handleEvent();

        // Update changed parameters
        if (mMenu.isOpen())
            unloadChangesQueue();

        // Update assets if there is a request
        if (ParameterLine::resetRefreshState())
            resetAssets();
    }

    if (type & UpdateType::Hooks)
    {
        // Take buttons realtime input
        for (auto &button : mButtons)
            button->processInput();
    }

    if (type & UpdateType::Event)
    {
        if (!Settings::WindowTitleBar)
            moveWindow();

        // Make separate windows handle own events
        if (mMenu.isOpen())
            mMenu.processInput();
        if (mGfxButtonSelector->isOpen())
            mGfxButtonSelector->handleOwnInput();
        if (mKPSWindow->isOpen())
            mKPSWindow->handleOwnEvent();
        if (mGraph->isOpen())
            mGraph->handleOwnEvent();
    }
}

void Application::handleEvent()
{
    // SFML 3: pollEvent returns std::optional
    while (const std::optional event = mWindow.pollEvent())
    {
        // SFML 3: Type safe variant checks using getIf<>
        if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            const auto button = mouseButtonPressed->button;
            if (button == sf::Mouse::Button::Right) // Scoped Enum
            {
                auto idx = 0u;
                if (isPressPerformedOnButton(idx))
                {
                    mGfxButtonSelector->setKey(mButtons[idx]->getLogKey());
                    mGfxButtonSelector->openWindow();
                }
            }
        }
        
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            auto btnAmtChanged = false;
            
            if (Settings::KeyToIncreaseKeys.isTriggered(keyPressed) || Settings::AltKeyToIncreaseKeys.isTriggered(keyPressed))
            {
                addButton(*new LogKey("A", "A", new sf::Keyboard::Scancode(sf::Keyboard::Scancode::A), nullptr));
                btnAmtChanged = true;
            }

            if (Settings::KeyToIncreaseButtons.isTriggered(keyPressed))
            {
                addButton(*new LogKey("M Left", "M Left", nullptr, new sf::Mouse::Button(sf::Mouse::Button::Left)));
                btnAmtChanged = true;
            }

            if (Settings::KeyToDecreaseKeys.isTriggered(keyPressed) || Settings::AltKeyToDecreaseKeys.isTriggered(keyPressed) || Settings::KeyToDecreaseButtons.isTriggered(keyPressed))
            {
                removeButton();
                btnAmtChanged = true;
            }

            if (btnAmtChanged)
            {
                (*mButtonsPositioner)();
                (*mStatisticsPositioner)();
                resizeWindow();
                mBackground->rescale();
            }

            if (Settings::KeyToOpenKPSWindow.isTriggered(keyPressed))
            {
                if (mKPSWindow->isOpen())
                    mKPSWindow->closeWindow();
                else
                    mKPSWindow->openWindow();
            }

            if (Settings::KeyToOpenMenuWindow.isTriggered(keyPressed))
            {
                if (mMenu.isOpen())
                    mMenu.closeWindow();
                else
                    mMenu.openWindow();
            }

            if (Settings::KeyToReset.isTriggered(keyPressed))
            {
                for (auto &button : mButtons)
                    button->reset();
            }

            if (Settings::KeyExit.isTriggered(keyPressed))
            {
                mMenu.saveConfig(mButtons);
                mWindow.close();
                return;
            }
        }

        if (event->is<sf::Event::Closed>())
        {
            mMenu.saveConfig(mButtons);
            mWindow.close();
            return;
        }
    }
}

void Application::update(float deltaSeconds, UpdateType type)
{
    if (type & UpdateType::Event)
    {
        for (auto &button : mButtons)
            button->update(deltaSeconds);
        for (auto &line : mStatistics)
            line->update();

        if (mMenu.isOpen())
            mMenu.update();

        if (mKPSWindow->isOpen())
            mKPSWindow->update();
    }

    if (type & UpdateType::Hooks)
    {
        Button::moveIndex();
        for (auto &button : mButtons)
            button->accumulateBeatsPerMinute();
    }
}

void Application::render()
{
    mWindow.clear();

    mWindow.draw(*mBackground);

    for (const auto &elem : mButtons)
        mWindow.draw(*elem);
    for (const auto &elem : mStatistics)
        mWindow.draw(*elem);

    if (mMenu.isOpen())
        mMenu.render();
    if (mGfxButtonSelector->isOpen())
        mGfxButtonSelector->render();
    if (mKPSWindow->isOpen())
        mKPSWindow->render();
    if (mGraph->isOpen())
        mGraph->render();
    
    mWindow.display();
}

void Application::unloadChangesQueue()
{
    auto &queue = mMenu.getChangedParametersQueue();
    while (!queue.isEmpty())
    {
        auto pair = queue.pop();

        if (GfxStatisticsLine::parameterIdMatches(pair.first))
        {
            for (auto &line : mStatistics)
                line->updateParameters();
            (*mStatisticsPositioner)();
        }

        if (Button::parameterIdMatches(pair.first))
        {
            for (auto &button : mButtons)
            {
                button->updateParameters();
            }
            (*mButtonsPositioner)();
        }

        if (KPSWindow::parameterIdMatches(pair.first))
        {
            mKPSWindow->updateParameters();
        }

        if (parameterIdMatches(pair.first))
        {
            // SFML 3: setSize and FloatRect require braced vectors
            mWindow.setSize(sf::Vector2u(getWindowWidth(), getWindowHeight()));
            mWindow.setView(sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(mWindow.getSize().x), static_cast<float>(mWindow.getSize().y)})));
            mMenu.requestFocus();
        }

        if (pair.first == LogicalParameter::ID::MainWndwTitleBar)
        {
            openWindow();
        }

        if (pair.first == LogicalParameter::ID::RenderUpdateFrequency)
        {
            mWindow.setFramerateLimit(getApplicationUpdateFrequency());
        }

        mBackground->rescale();
    }
}

void Application::resetAssets()
{
    mFonts.clear();
    mTextures.clear();

    loadTextures();
    loadFonts();

    auto idx = 0u;
    for (auto &button : mButtons)
    {
        button->updateAssets();
        // SFML 3: setPosition requires a single vector
        button->setPosition({Button::getWidth(idx), Button::getHeight(idx)});
        ++idx;
    }

    for (auto &line : mStatistics)
        line->updateAsset();

    mBackground->updateAssets();
    (*mButtonsPositioner)();
    (*mStatisticsPositioner)();

    mMenu.saveConfig(mButtons);
}

void Application::loadTextures()
{
    if (!mTextures.loadFromFile(Textures::Button, Settings::GfxButtonTexturePath))
        mTextures.loadFromMemory(Textures::Button, Settings::DefaultButtonTexture, Settings::DefaultButtonTextureSize);

    if (!mTextures.loadFromFile(Textures::Animation, Settings::AnimationTexturePath))
        mTextures.loadFromMemory(Textures::Animation, Settings::DefaultAnimationTexture, Settings::DefaultAnimationTextureSize);

    Settings::isGreenscreenSet = Settings::BackgroundTexturePath == "GreenscreenBG.png";
    if (Settings::isGreenscreenSet)
        mTextures.loadFromMemory(Textures::Background, Settings::DefaultGreenscreenBackgroundTexture, Settings::DefaultGreenscreenBackgroundTextureSize);
    else
    {
        if (!mTextures.loadFromFile(Textures::Background, Settings::BackgroundTexturePath))
            mTextures.loadFromMemory(Textures::Background, Settings::DefaultBackgroundTexture, Settings::DefaultBackgroundTextureSize);
    }
}

void Application::loadFonts()
{
    if (!mFonts.loadFromFile(Fonts::ButtonValue, Settings::ButtonTextFontPath))
        mFonts.loadFromMemory(Fonts::ButtonValue, Settings::KeyCountersDefaultFont, Settings::KeyCountersDefaultFontSize);

    if (!mFonts.loadFromFile(Fonts::Statistics, Settings::StatisticsTextFontPath))
        mFonts.loadFromMemory(Fonts::Statistics, Settings::StatisticsDefaultFont, Settings::StatisticsDefaultFontSize);

    if (!mFonts.loadFromFile(Fonts::KPSText, Settings::KPSWindowTextFontPath))
        mFonts.loadFromMemory(Fonts::KPSText, Settings::DefaultKPSWindowFont, Settings::DefaultKPSWindowFontSize);

    if (!mFonts.loadFromFile(Fonts::KPSNumber, Settings::KPSWindowNumberFontPath))
        mFonts.loadFromMemory(Fonts::KPSNumber, Settings::DefaultKPSWindowFont, Settings::DefaultKPSWindowFontSize);
}

void Application::loadIcon()
{
    sf::Image icon;
    (void)icon.loadFromMemory(IconTexture, IconTexture_size); // Cast to void to silence nodiscard warning
    mWindow.setIcon({256, 256}, icon.getPixelsPtr());
}

void Application::buildStatistics()
{
    using Ptr = std::unique_ptr<GfxStatisticsLine>;
    auto linePtr = Ptr();
    auto id = static_cast<unsigned>(GfxStatisticsLine::StatisticsID::KPS);
    
    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsKPS, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;

    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsTotal, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;

    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsBPM, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;
}

void Application::buildButtons()
{
    auto logKeyQueue = ConfigHelper::oldGetLogKeys();
    auto logKeyBtnsQueue = ConfigHelper::oldGetLogButtons();

    while (logKeyBtnsQueue.size())
    {
        logKeyQueue.push(logKeyBtnsQueue.front());
        logKeyBtnsQueue.pop();
    }
    if (logKeyQueue.empty())
        logKeyQueue = ConfigHelper::getLogKeys();

    while (!logKeyQueue.empty())
    {
        addButton(logKeyQueue.front());
        logKeyQueue.pop();
    }
    
    // TODO remove *new LogKey, use smart ptrs instead
    if (mButtons.empty())
    {
        // Migrated to Scancode explicitly
        addButton(*new LogKey("Z", "Z", new sf::Keyboard::Scancode(sf::Keyboard::Scancode::Z), nullptr));
        addButton(*new LogKey("X", "X", new sf::Keyboard::Scancode(sf::Keyboard::Scancode::X), nullptr));
    }
}

bool Application::isPressPerformedOnButton(unsigned &btnIdx) const
{
    const auto size = Button::size();
    for (auto i = 0ul; i < size; ++i)
    {
        if (isMouseInRange(static_cast<unsigned>(i)))
        {
            btnIdx = static_cast<unsigned>(i);
            return true;
        }
    }
    return false;
}

bool Application::isMouseInRange(unsigned idx) const
{
    auto mousePosI = sf::Mouse::getPosition(mWindow);
    const auto mousePosition = sf::Vector2f(static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y));
    const auto textureSize = static_cast<sf::Vector2f>(Settings::GfxButtonTextureSize);
    const auto &button = *mButtons[idx];
    const auto buttonPosition = button.getPosition() - textureSize / 2.f;
    
    // SFML 3: FloatRect strictly requires position and size vectors
    const auto buttonRectangle = sf::FloatRect(buttonPosition, textureSize);

    return buttonRectangle.contains(mousePosition);
}

void Application::addButton(LogKey &logKey)
{
    mButtons.emplace_back(std::make_unique<Button>(logKey, mTextures, mFonts));
}

void Application::removeButton()
{
    mButtons.pop_back();
}

void Application::openWindow()
{
    std::uint32_t style;
#ifdef _WIN32
    style = Settings::WindowTitleBar ? sf::Style::Close : sf::Style::None;
#elif __linux__
    style = Settings::WindowTitleBar ? sf::Style::Default : sf::Style::None;
#else
#error Unsupported compiler
#endif

    if (mWindow.isOpen())
        mWindow.close();
        
    // SFML 3: VideoMode takes a vector
    mWindow.create(sf::VideoMode({getWindowWidth(), getWindowHeight()}), "JKPS", style);
    mWindow.setKeyRepeatEnabled(false);
    mWindow.setFramerateLimit(getApplicationUpdateFrequency());
#ifdef linux
    if (style == sf::Style::None)
    {
        auto desktop = sf::VideoMode::getDesktopMode();
        auto windowSize = static_cast<sf::Vector2i>(mWindow.getSize());
        mWindow.setPosition(sf::Vector2i(
            desktop.size.x  / 2 - windowSize.x / 2, 
            desktop.size.y / 2 - windowSize.y / 2));
    }
#endif
}

void Application::resizeWindow()
{
    const auto size = sf::Vector2u(getWindowWidth(), getWindowHeight());
    mWindow.setSize(size);

    auto windowSize = static_cast<sf::Vector2f>(mWindow.getSize());
    auto view = sf::View(sf::FloatRect({0.f, 0.f}, windowSize));
    mWindow.setView(view);
}

void Application::moveWindow()
{
    static auto mLastMousePosition = sf::Vector2i();
    if (mWindow.hasFocus() && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) // Scoped enum
    {
        mWindow.setPosition(mWindow.getPosition() + 
            sf::Mouse::getPosition() - mLastMousePosition);
    }
    mLastMousePosition = sf::Mouse::getPosition();
}

unsigned Application::getWindowWidth()
{
    const auto btnAmt = static_cast<float>(Button::size());
    const auto width = static_cast<int>(
        static_cast<float>(Settings::GfxButtonTextureSize.x) * btnAmt +
        (btnAmt - 1.f) * Settings::GfxButtonDistance +
        static_cast<float>(Settings::WindowBonusSizeLeft) + static_cast<float>(Settings::WindowBonusSizeRight));

    return static_cast<unsigned>(std::max(5, width));
}

unsigned Application::getWindowHeight()
{
    const auto height = static_cast<int>(
        static_cast<float>(Settings::GfxButtonTextureSize.y) + static_cast<float>(Settings::WindowBonusSizeTop) +
        static_cast<float>(Settings::WindowBonusSizeBottom));

    return static_cast<unsigned>(std::max(5, height));
}

sf::IntRect Application::getWindowRect()
{
    // SFML 3: IntRect requires {position}, {size}
    return { {0, 0}, {static_cast<int>(getWindowWidth()), static_cast<int>(getWindowHeight())} };
}

bool Application::parameterIdMatches(LogicalParameter::ID id)
{
    return
        id == LogicalParameter::ID::BtnGfxTxtrSz ||  
        id == LogicalParameter::ID::BtnTextChSz ||
        id == LogicalParameter::ID::BtnGfxDist  ||
        id == LogicalParameter::ID::MainWndwTop ||
        id == LogicalParameter::ID::MainWndwBot ||
        id == LogicalParameter::ID::MainWndwLft ||
        id == LogicalParameter::ID::MainWndwRght;
}

unsigned Application::getRenderUpdateFrequency() const
{
    return Settings::RenderUpdateFrequency;
}

unsigned Application::getApplicationUpdateFrequency() const
{
    return std::max(Settings::RenderUpdateFrequency, HooksUpdateFrequency);
}