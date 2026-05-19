#include "../Headers/GfxButton.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/Button.hpp"
#include "../Headers/Application.hpp"
#include "../Headers/Utility.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <algorithm>
#include <cstdint>


bool GfxButton::mShowBounds(false);
int GfxButton::mSelectedKeyBounds(-1);

GfxButton::GfxButton(const unsigned idx, const TextureHolder &textureHolder, const FontHolder &fontHolder)
: mTextures(textureHolder)
, mFonts(fontHolder)
, mEmitter(idx)
, mLastKeyState(false)
, mButtonsHeightOffset(0.f)
, mBtnIdx(idx)
{
    // SFML 3: Sprite now requires a texture on construction
    mSprites[ButtonSprite] = std::make_unique<sf::Sprite>(mTextures.get(Textures::Button));
    mSprites[AnimationSprite] = std::make_unique<sf::Sprite>(mTextures.get(Textures::Animation));

    // SFML 3: Text now requires a font on construction
    const auto& font = mFonts.get(Fonts::ButtonValue);
    for (unsigned i = 0; i < TextIdCounter; ++i)
    {
        mTexts[static_cast<TextID>(i)] = std::make_unique<sf::Text>(font);
    }

    updateAssets();
    updateParameters();

    mBounds.setFillColor(sf::Color::Transparent);
    mBounds.setOutlineColor(sf::Color::Magenta);
    mBounds.setOutlineThickness(1.f);
}

void GfxButton::update(float deltaSeconds, bool keyState)
{
    if (Settings::LightAnimation)
        keyState ? lightKey() : fadeKey();
    if (Settings::PressAnimation)
        keyState ? lowerKey() : raiseKey();

    if (Settings::KeyPressVisToggle)
    {
        // Create a new rectangle on button press if last frame the button was not pressed
        if (!mLastKeyState && keyState)
        {
            const auto &buttonSprite = *mSprites[ButtonSprite];
            const auto rect = buttonSprite.getGlobalBounds();
            // SFML 3: rect.width and rect.height became rect.size
            mEmitter.create(deltaSeconds, rect.size);
        }
    }

    mEmitter.update(deltaSeconds, keyState, mLastKeyState);

    mLastKeyState = keyState;
}

void GfxButton::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto drawText = [this, &target] (const sf::Text &text, sf::RenderStates states)
        {
            target.draw(text, states);
            const auto boundsStates = states.transform.translate(text.getPosition());

            if (mShowBounds && (mSelectedKeyBounds == -1 || mSelectedKeyBounds == mBtnIdx))
            {
                target.draw(mBounds, boundsStates);
            }
        };
    states.transform *= getTransform();

    // Nullify scaling for emitter draw
    auto emitterStates = states;
    
    auto inverseScale = sf::Transform::Identity;
    inverseScale.scale(getScale());
    inverseScale = inverseScale.getInverse();

    emitterStates.transform *= inverseScale;

    // Key visualizer's graphics
    target.draw(mEmitter, emitterStates);

    // Key's graphics
    for (const auto &sprite : mSprites)
        target.draw(*sprite, states);

    // Key's text
    if (Settings::ButtonTextShowVisualKeys) 
        drawText(*mTexts[VisualKey], states);
    if (Settings::ButtonTextShowTotal)
        drawText(*mTexts[KeyCounter], states);
    if (Settings::ButtonTextShowKPS)
        drawText(*mTexts[KeyPerSecond], states);
    if (Settings::ButtonTextShowBPM)
        drawText(*mTexts[BeatsPerMinute], states);

}

void GfxButton::lightKey()
{
    mSprites[AnimationSprite]->setColor(Settings::AnimationColor);
    // AnimationScale is already a vector, pass it directly
    setScale(Settings::AnimationScale / 100.f); 
}

void GfxButton::fadeKey()
{
    auto &animationSprite = *mSprites[AnimationSprite];
    const auto color = animationSprite.getColor();
    auto scale = getScale();
    if (scale.x == 1.f && scale.y == 1.f && color.a == 0)
        return;

    const sf::Color animationStep(0, 0, 0, static_cast<std::uint8_t>(255 / Settings::AnimationFrames));
    const auto scaleStep = getScaleStep();

    // Prevent implicit conversion warning by explicitly casting the subtraction
    animationSprite.setColor(sf::Color(color.r, color.g, color.b, static_cast<std::uint8_t>(color.a - animationStep.a)));
    setScale(scale + scaleStep);

    scale = getScale();
    // Use explicit sf::Vector2f to avoid ambiguity with the new Angle constructor
    if ((scaleStep.x > 0.f && scale.x > 1.f) || (scaleStep.x < 0 && scale.x < 1.f))
        setScale(sf::Vector2f(1.f, getScale().y));
    if ((scaleStep.y > 0.f && scale.y > 1.f) || (scaleStep.y < 0 && scale.y < 1.f))
        setScale(sf::Vector2f(getScale().x, 1.f));
}

void GfxButton::lowerKey()
{
    if (mButtonsHeightOffset == Settings::AnimationOffset)
        return;

    for (auto &sprite : mSprites)
    {
        const auto position = sprite->getPosition();
        // SFML 3: setPosition requires a vector
        sprite->setPosition({position.x, position.y + Settings::AnimationOffset - mButtonsHeightOffset});
    }
    if (!Settings::ButtonTextIgnoreBtnMovement)
    {
        for (auto &label : mTexts)
        {
            const auto position = label->getPosition();
            label->setPosition({position.x, position.y + Settings::AnimationOffset - mButtonsHeightOffset});
        }
    }
    mButtonsHeightOffset = Settings::AnimationOffset;
}

void GfxButton::raiseKey()
{
    if (mButtonsHeightOffset <= 0.f)
        return;

    const auto step = std::min(getRiseStep(), mButtonsHeightOffset);

    for (auto &sprite : mSprites)
    {
        const auto position = sprite->getPosition();
        sprite->setPosition({position.x, position.y - step});
    }
    if (!Settings::ButtonTextIgnoreBtnMovement)
    {
        for (auto &label : mTexts)
        {
            const auto position = label->getPosition();
            label->setPosition({position.x, position.y - step});
        }
    }
    mButtonsHeightOffset = std::max(mButtonsHeightOffset - step, 0.f);
}

sf::Vector2f GfxButton::getScaleStep() const
{
    return (sf::Vector2f(1.f, 1.f) - Settings::AnimationScale / 100.f) / static_cast<float>(Settings::AnimationFrames);
}

float GfxButton::getRiseStep() const
{
    return Settings::AnimationOffset / Settings::AnimationFrames;
}

void GfxButton::updateAssets()
{
    resetAssets();
    scaleSprites();
    centerOrigins();
}

void GfxButton::updateParameters()
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto sepValAdvMode = isInSupportedRange && Settings::ButtonTextSepPosAdvancedMode;
    const auto advTextMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto advGfxMode = isInSupportedRange && Settings::GfxButtonAdvancedMode;

    const auto color = !advGfxMode ? Settings::GfxButtonTextureColor : Settings::GfxButtonsColor[mBtnIdx];

    scaleSprites();
    mSprites[ButtonSprite]->setColor(color);
    
    // Substraction by black (0,0,0,255) drops alpha to 0. It is safer to just modify alpha directly.
    auto animColor = Settings::AnimationColor;
    animColor.a = 0;
    mSprites[AnimationSprite]->setColor(animColor);

    auto idx = 0ul;
    for (auto &text : mTexts)
    {
        const auto color = !advTextMode ? Settings::ButtonTextColor : Settings::ButtonTextAdvColor[mBtnIdx];
        const auto chSz = !advTextMode ? Settings::ButtonTextCharacterSize : Settings::ButtonTextAdvCharacterSize[mBtnIdx];
        const auto outThck = (!advTextMode ? Settings::ButtonTextOutlineThickness : Settings::ButtonTextAdvOutlineThickness[mBtnIdx]) / 10.f;
        const auto outColor = !advTextMode ? Settings::ButtonTextOutlineColor : Settings::ButtonTextAdvOutlineColor[mBtnIdx];
        const auto bold = !advTextMode ? Settings::ButtonTextBold : Settings::ButtonTextAdvBold[mBtnIdx];
        const auto italic = !advTextMode ? Settings::ButtonTextItalic : Settings::ButtonTextAdvItalic[mBtnIdx];
        const auto advPos = Settings::ButtonsTextAdvPosition[mBtnIdx];
        const auto origPos = Utility::swapY(Settings::ButtonTextPosition + (advTextMode ? advPos : sf::Vector2f()));
        auto pos = origPos;

        switch(idx)
        {
            case VisualKey:
                pos += Utility::swapY(Settings::ButtonTextVisualKeysTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvVisualKeysTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case KeyCounter:
                pos += Utility::swapY(Settings::ButtonTextTotalTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvTotalTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case KeyPerSecond:
                pos += Utility::swapY(Settings::ButtonTextKPSTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvKPSTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case BeatsPerMinute:
                pos += Utility::swapY(Settings::ButtonTextBPMTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvBPMTextPosition[mBtnIdx] : sf::Vector2f()));
                break;
        }

        text->setFillColor(color);
        text->setCharacterSize(chSz);
        text->setPosition(pos);
        
        // SFML 3: Scoped enums for text styling
        std::uint32_t style = static_cast<std::uint32_t>(sf::Text::Style::Regular);
        if (bold) style |= static_cast<std::uint32_t>(sf::Text::Style::Bold);
        if (italic) style |= static_cast<std::uint32_t>(sf::Text::Style::Italic);
        text->setStyle(style);
        
        text->setOutlineThickness(outThck);
        text->setOutlineColor(outColor);

        // SFML 3: LAlt is a scoped enum now
        const auto lAlt = Settings::ShowOppOnAlt && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt);
        if ((lAlt && idx == VisualKey && Settings::ButtonTextShowVisualKeys && !Settings::ButtonTextShowTotal)
        || (lAlt && idx == KeyCounter && !Settings::ButtonTextShowVisualKeys && Settings::ButtonTextShowTotal))
        {
            text->setPosition(pos);
        }

        ++idx;
    }

    const auto size = !advTextMode ? Settings::ButtonTextBounds : Settings::ButtonTextAdvBounds[mBtnIdx];
    mBounds.setSize(size);
    mBounds.setOrigin(size / 2.f);
}

void GfxButton::resetAssets()
{
    mSprites[ButtonSprite]->setTexture(mTextures.get(Textures::Button), true);
    mSprites[AnimationSprite]->setTexture(mTextures.get(Textures::Animation), true);

    const auto &font = mFonts.get(Fonts::ButtonValue);
    for (auto &text : mTexts)
        text->setFont(font);
}

void GfxButton::scaleSprites()
{
    auto &buttonSprite = *mSprites[ButtonSprite];
    auto &animationSprite = *mSprites[AnimationSprite];

    // SFML 3: getTexture() returns a const reference
    const auto origBtnTxtrSz = buttonSprite.getTexture().getSize();
    const auto origAniTxtrSz = animationSprite.getTexture().getSize();

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::GfxButtonAdvancedMode;

    const auto btnTxtrSz = !advMode ? static_cast<sf::Vector2f>(Settings::GfxButtonTextureSize) : Settings::GfxButtonsSizes[mBtnIdx];
    const auto btnTxtrScale = sf::Vector2f(btnTxtrSz.x / static_cast<float>(origBtnTxtrSz.x), btnTxtrSz.y / static_cast<float>(origBtnTxtrSz.y));
    const auto aniTxtrScale = sf::Vector2f(btnTxtrSz.x / static_cast<float>(origAniTxtrSz.x), btnTxtrSz.y / static_cast<float>(origAniTxtrSz.y));

    buttonSprite.setScale(btnTxtrScale);
    animationSprite.setScale(aniTxtrScale);
}

bool isInBounds(sf::Vector2f bounds, sf::FloatRect rect)
{
    // SFML 3: sf::Rect nested values
    return rect.size.x > bounds.x || rect.size.y > bounds.y;
}

void GfxButton::keepInBounds(sf::Text &text)
{
    if (!Settings::ButtonTextBoundsToggle)
        return;
        
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto bounds = !advMode ? Settings::ButtonTextBounds : Settings::ButtonTextAdvBounds[mBtnIdx];
    auto rect = text.getLocalBounds();

    for (auto chSz = text.getCharacterSize(); 
        chSz > 2u && isInBounds(bounds, rect); 
        --chSz)
    {
        text.setCharacterSize(chSz);
        rect = text.getLocalBounds();
    }
}

sf::Vector2f getTextCenter(const sf::Text &text)
{
    const auto rect = text.getLocalBounds();
    // SFML 3: sf::Rect structure changed to position and size subvectors
    return { rect.position.x + rect.size.x / 2.f, rect.position.y + rect.size.y / 2.f};
}

void GfxButton::centerOrigins()
{
    auto &buttonSprite = *mSprites[ButtonSprite];
    auto &animationSprite = *mSprites[AnimationSprite];
    // SFML 3: getTexture returns reference
    const auto buttonTextureSize = static_cast<sf::Vector2f>(buttonSprite.getTexture().getSize());
    const auto animationTextureSize = static_cast<sf::Vector2f>(animationSprite.getTexture().getSize());
    buttonSprite.setOrigin(buttonTextureSize / 2.f);
    animationSprite.setOrigin(animationTextureSize / 2.f);

    for (auto &text : mTexts)
        text->setOrigin(getTextCenter(*text));
}

float GfxButton::getWidth(unsigned idx)
{
    const float width = Settings::WindowBonusSizeLeft + 
        (Settings::GfxButtonTextureSize.x + Settings::GfxButtonDistance) * idx + 
        Settings::GfxButtonTextureSize.x / 2;
    return width;
}

float GfxButton::getHeight(unsigned idx)
{
    const float height = Settings::WindowBonusSizeTop + Settings::GfxButtonTextureSize.y / 2;
    return height;
}

GfxButton::TextID GfxButton::getTextIdToDisplay()
{
    if (Settings::ButtonTextShowVisualKeys) 
        return VisualKey;
    if (Settings::ButtonTextShowTotal)
        return KeyCounter;
    if (Settings::ButtonTextShowKPS)
        return KeyPerSecond;
    if (Settings::ButtonTextShowBPM)
        return BeatsPerMinute;
    
    return Nothing;
}

void GfxButton::setShowBounds(bool flag, int idx)
{
    mShowBounds = flag;
    mSelectedKeyBounds = idx;
}

void GfxButton::setNextBarColor(sf::Color color)
{
    mEmitter.setNextBarColor(color);
}

GfxButton::~GfxButton()
{    
}

// SFML 3: sf::Quads was removed. Migrating directly to Triangles.
GfxButton::RectEmitter::RectEmitter(unsigned btnIdx)
: mBtnIdx(btnIdx)
, mMiddleVertecies(sf::PrimitiveType::Triangles, 1500u) // 250 rects * 6 vertices
, mNextBarColor(sf::Color::White)
{
    const auto count = mMiddleVertecies.getVertexCount() / 6;
    mBarColors.assign(count, sf::Color::White);
    for (auto i = 0ul; i < count; ++i)
        mAvailableRectIndices.emplace_back(i);
}

void GfxButton::RectEmitter::update(float deltaSeconds, bool keyState, bool prevKeyState)
{
    if (mUsedRectIndices.empty())
        return;

    std::vector<size_t> toRemove;

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];
    const auto minHeight = !advMode ? Settings::KeyPressFixedHeight :
        Settings::KeyPressAdvFixedHeight[mBtnIdx];

    for (auto i : mUsedRectIndices)
    {
        auto eachVertexIsOnLimit = true;
        const auto vertexIndex = i * 6ul;
        for (auto j = vertexIndex; j < vertexIndex + 6ul; ++j)
        {
            auto &middleVertex = mMiddleVertecies[j];

            auto move = [len, speed, deltaSeconds] (sf::Vertex &vertex)
                {
                    vertex.position.y = -std::min(std::abs(vertex.position.y + speed * deltaSeconds * getConstantSpeedScale()), len);
                };
            
            move(middleVertex);

            if (eachVertexIsOnLimit)
            {
                eachVertexIsOnLimit = std::abs(middleVertex.position.y) == len;
            }

            middleVertex.color = getVertexColor(mMiddleVertecies, j);
        }

        if (eachVertexIsOnLimit)
        {
            mAvailableRectIndices.emplace_back(i);
            toRemove.emplace_back(i);
        }
    }

    for (auto i : toRemove)
    {
        mUsedRectIndices.erase(std::remove(
                mUsedRectIndices.begin(), mUsedRectIndices.end(), i), 
            mUsedRectIndices.end());
    }

    if (keyState)
    {
        const auto offset = mUsedRectIndices.back() * 6ul;
        const auto delta = speed * deltaSeconds * getConstantSpeedScale();
        
        mMiddleVertecies[offset + 2ul].position.y -= delta;
        mMiddleVertecies[offset + 3ul].position.y -= delta;
        mMiddleVertecies[offset + 4ul].position.y -= delta;

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            const float newY = mMiddleVertecies[offset].position.y + minHeight;
            mMiddleVertecies[offset + 2ul].position.y = newY;
            mMiddleVertecies[offset + 3ul].position.y = newY;
            mMiddleVertecies[offset + 4ul].position.y = newY;
        }
    }

    if (prevKeyState && !keyState && !mUsedRectIndices.empty())
    {
        const auto offset = mUsedRectIndices.back() * 6ul;
        const auto delta = speed * deltaSeconds * getConstantSpeedScale();
        
        mMiddleVertecies[offset + 2ul].position.y -= delta;
        mMiddleVertecies[offset + 3ul].position.y -= delta;
        mMiddleVertecies[offset + 4ul].position.y -= delta;

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            const float newY = mMiddleVertecies[offset].position.y + minHeight;
            mMiddleVertecies[offset + 2ul].position.y = newY;
            mMiddleVertecies[offset + 3ul].position.y = newY;
            mMiddleVertecies[offset + 4ul].position.y = newY;
        }
    }
}

void GfxButton::RectEmitter::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform = getPressRectTransform(states.transform);
    target.draw(mMiddleVertecies, states);
}

void GfxButton::RectEmitter::setPosition(sf::Vector2f position)
{
    mEmitterPosition = position;
}

void GfxButton::RectEmitter::pushVertecies(sf::VertexArray &vertexArray, sf::Vertex *toPush, size_t offset, sf::Vector2f buttonSize)
{
    for (auto i = 0ul; i < 6ul; ++i)
    {
        auto &vertex = toPush[i];
        vertex.position += mEmitterPosition - sf::Vector2f(0.f, buttonSize.y / 2.f);
        const auto idx = offset + i;
        vertex.color = getVertexColor(vertexArray, idx);
        vertexArray[idx] = vertex;
    }
}

float GfxButton::RectEmitter::getConstantSpeedScale()
{
    return 60.f;
}

void GfxButton::RectEmitter::create(float deltaSeconds, sf::Vector2f buttonSize)
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;

    const auto rectSize = sf::Vector2f(buttonSize.x, speed);
    const auto halfRectSize = rectSize / 2.f;

    const auto rectIndex = mAvailableRectIndices.back();
    mBarColors[rectIndex] = mNextBarColor;  // bake color at bar creation
    const auto firstVertexIndex = rectIndex * 6ul;

    // SFML 3: mapped 4 quad vertices to 6 triangle vertices
    sf::Vertex middleVertices[6];
    middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -rectSize.y);
    middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -rectSize.y);
    middleVertices[2].position = sf::Vector2f(+halfRectSize.x, 0.f);

    middleVertices[3].position = sf::Vector2f(+halfRectSize.x, 0.f);
    middleVertices[4].position = sf::Vector2f(-halfRectSize.x, 0.f);
    middleVertices[5].position = sf::Vector2f(-halfRectSize.x, -rectSize.y);

    pushVertecies(mMiddleVertecies, middleVertices, firstVertexIndex, buttonSize);
    
    mAvailableRectIndices.pop_back();
    mUsedRectIndices.emplace_back(rectIndex);
}

void GfxButton::RectEmitter::scaleTexture(sf::Vector2f buttonSize)
{
}

void GfxButton::RectEmitter::setNextBarColor(sf::Color color)
{
    mNextBarColor = color;
}

sf::Transform GfxButton::RectEmitter::getPressRectTransform(sf::Transform transform) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto rot = !advMode ? Settings::KeyPressVisRotation : 
        Settings::KeyPressVisAdvRotation[mBtnIdx];
    const auto orig = Settings::KeyPressVisOrig + (advMode 
        ? Settings::KeyPressVisAdvOrig[mBtnIdx] : sf::Vector2f());
    const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;

    // SFML 3: Rotation requires an sf::Angle
    transform.rotate(sf::degrees(-rot));
    transform.translate(Utility::swapY(orig));
    transform.scale({wScale, 1.f});
    return transform;
}

float GfxButton::RectEmitter::getVertexProgress(size_t vertexNumber, float vertexHeight) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];

    return std::min(mEmitterPosition.y - vertexHeight / len, 1.f);
}

sf::Color GfxButton::RectEmitter::getVertexColor(const sf::VertexArray &vertexArray, size_t vertexIndex) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;

    // osu!alt mode overrides the bar color (disables both normal and advanced color)
    sf::Color color;
    if (Settings::OsuAltMode && Settings::KeyPressVisToggle)
    {
        // Use the per-bar baked color so existing bars are never retroactively recolored
        color = mBarColors[vertexIndex / 6ul];
    }
    else
    {
        color = !advMode ? Settings::KeyPressVisColor :
            Settings::KeyPressVisAdvColor[mBtnIdx];
    }

    color.a -= static_cast<std::uint8_t>(color.a * getVertexProgress(vertexIndex, vertexArray[vertexIndex].position.y));
    return color;
}