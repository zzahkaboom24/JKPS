#include "../Headers/GfxParameter.hpp"
#include "../Headers/StringHelper.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/ParameterLine.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cassert>


const sf::Color GfxParameter::defaultRectColor(120, 120, 120);
const sf::Color GfxParameter::defaultAimedRectColor(160, 160, 160);
const sf::Color GfxParameter::defaultSelectedRectColor(180, 180, 180);
const TextureHolder *GfxParameter::mTextures = nullptr;
const FontHolder *GfxParameter::mFonts = nullptr;

// All types, except Bool
GfxParameter::GfxParameter(const sf::Font *font, const std::string &str, unsigned n, sf::Vector2f rectSize)
: mParent(nullptr)
{
    assert(font);

    mRect.setSize(rectSize);
    mRect.setOrigin(mRect.getSize() / 2.f);
    mRect.setFillColor(defaultRectColor);

    mValText.emplace(*font);
    mValText->setString(str);
    setupValPos();

    float distance = 80.f;
    setPosition({distance * static_cast<float>(n), 0.f});
}

// Bool type
GfxParameter::GfxParameter(const ParameterLine *parent, bool b)
: mParent(parent)
{
    assert(mFonts);
    mValText.emplace(mFonts->get(Fonts::Value));
    mValText->setString(b ? "True" : "False");
    setRightTexture();
    mSprite->setOrigin(static_cast<sf::Vector2f>(mSprite->getTexture().getSize()) / 2.f);
}

// Tab
GfxParameter::GfxParameter(const std::string &str, sf::Vector2f rectSize)
: mParent(nullptr)
{
    mRect.setSize(rectSize);
    mRect.setOrigin(rectSize / 2.f);
    mRect.setFillColor(defaultRectColor);

    assert(mFonts);
    mValText.emplace(mFonts->get(Fonts::Value));
    mValText->setString(str);
    mValText->setCharacterSize(15);

    auto rect = mValText->getLocalBounds();
    mValText->setOrigin({
        rect.position.x + rect.size.x / 2.f,
        rect.position.y + rect.size.y / 2.f});
}

GfxParameter::GfxParameter(const ParameterLine *parent)
: mParent(parent)
{
    assert(mTextures);
    mSprite.emplace(mTextures->get(Textures::Refresh));
    mSprite->setOrigin(static_cast<sf::Vector2f>(mSprite->getTexture().getSize()) / 2.f);
}

void GfxParameter::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    if (mRect.getSize().x != 0)
    {
        target.draw(mRect, states);
        if (mValText)
            target.draw(*mValText, states);
    }
    else if (mSprite)
    {
        target.draw(*mSprite, states);
    }
}

void GfxParameter::setupValPos()
{
    if (!mValText) return;
    mValText->setCharacterSize(20);
    mValText->setOrigin({
        mValText->getLocalBounds().position.x + mValText->getLocalBounds().size.x  / 2.f,
        mValText->getLocalBounds().position.y  + mValText->getLocalBounds().size.y / 2.f});
}


float GfxParameter::getPosX()
{
    float maxValues = 4.f, valRectWidth = 70.f, distBetweenEdges = 10.f, distBetweenOrigins = 80.f;
    return distBetweenOrigins * (maxValues - 1) + valRectWidth / 2 + distBetweenEdges;
}

bool GfxParameter::contains(sf::Vector2f v2) const
{
    return getGlobalBounds().contains(v2);
}

sf::FloatRect GfxParameter::getGlobalBounds() const
{
    const sf::Vector2f size(mRect.getSize().x != 0.f ? 
        mRect.getSize() : sf::Vector2f(mSprite->getTexture().getSize()));
    return { getGlobalPosition() - size / 2.f, size };
}

sf::Vector2f GfxParameter::getGlobalPosition() const
{
    return getPosition() + (mParent ? mParent->getPosition() : sf::Vector2f({0.f,0.f}));
}

void GfxParameter::setInverseMark()
{
    setRightTexture();
}

void GfxParameter::setRightTexture()
{
    assert(mTextures);
    if (!mValText) return;
    std::string str = static_cast<std::string>(mValText->getString());
    if (str == "True" || str == "true" || str == "TRUE")
        mSprite.emplace(mTextures->get(Textures::vMark));
    else
        mSprite.emplace(mTextures->get(Textures::xMark));
}
