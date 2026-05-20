#include "../Headers/KeysPerSecondGraph.hpp"
#include "../Headers/Button.hpp"
#include "../Headers/Settings.hpp"

#include <SFML/Window/Event.hpp>

#include <iostream>
#include <stdlib.h>
#include <optional>


KeysPerSecondGraph::KeysPerSecondGraph()
: mVertecies(sf::PrimitiveType::TriangleFan, 16)
, mActiveVertecies(16)
{
    srand(static_cast<unsigned>(time(NULL)));

    const float widthStep = 800.f / 13.f;
    float width = 0.f;
    float height = 600.f;
    for (unsigned i = 0; i < 14; ++i)
    {
        sf::Vertex &vertex = mVertecies[i];
        vertex.position.x = width;
        vertex.position.y = height - static_cast<float>(i * 20u);
        width += widthStep;
    }
    mVertecies[14].position = sf::Vector2f(800, 600);
    mVertecies[15].position = sf::Vector2f(0, 600);
}

void KeysPerSecondGraph::handleOwnEvent()
{
    while (const std::optional event = mWindow.pollEvent())
    { 
        if (const auto* keyP = event->getIf<sf::Event::KeyPressed>())
        {
            if (Settings::KeyExit.isTriggered(keyP))
                mWindow.close();
        }

        if (event->is<sf::Event::Closed>())
        {
            mWindow.close();
        }
    }
}

void KeysPerSecondGraph::update()
{

}

void KeysPerSecondGraph::render()
{
    mWindow.clear(sf::Color(30, 30, 30));

    mWindow.draw(mVertecies);

    mWindow.display();
}

void KeysPerSecondGraph::openWindow()
{
    if (!mWindow.isOpen())
    {
        sf::ContextSettings settings;
        settings.antiAliasingLevel = 8;
        mWindow.create(sf::VideoMode({800u, 600u}), "Graph", sf::Style::Close, sf::State::Windowed, settings);
    }
}

void KeysPerSecondGraph::closeWindow()
{
    if (mWindow.isOpen())
        mWindow.close();
}

bool KeysPerSecondGraph::isOpen() const
{
    return mWindow.isOpen();
}

void KeysPerSecondGraph::updateParameters()
{
    // mActiveVertecies =
}

bool KeysPerSecondGraph::parameterIdMatches(LogicalParameter::ID id)
{
    (void)id;
    return false;
}
