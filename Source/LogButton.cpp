#include "../Headers/LogButton.hpp"
#include "../Headers/Settings.hpp"

#include <cassert>
#include <iostream>


unsigned LogButton::mBufferIndex(0);
unsigned LogButton::mPrevKpsBufferIndex(0);
float LogButton::statKeysPerSecond(0);
float LogButton::statMaxKeysPerSecond(0);
unsigned LogButton::statTotal(0);
float LogButton::statBeatsPerMinute(0);

// osu!alt mode shared state
static int sLastGroup(-1);           // last key group that was pressed (0 = left, 1 = right)
static bool sHeld[4] = {};           // whether each of the first 4 buttons is currently held

LogButton::LogButton(const unsigned idx, LogKey &key)
: mKey(key)
, mKeysPerSecond(0)
, mTotal(0)
, mBtnIdx(idx)
, mState(false)
, mPressAltColor(sf::Color::White)
, mLastAccumulateBpmBufferIndex(60u)
{
    statMaxKeysPerSecond = Settings::MaxKPS;
    statTotal = Settings::Total;
    for (auto &elem : mBuffer)
        elem = 0u;
    for (auto &elem : mPrevKpsBuffer)
        elem = 0.f;
}

void LogButton::processRealtimeInput()
{
    auto &bufferElem = mBuffer[mBufferIndex];
    const auto prevState = mState;
    mState = mKey.isPressed();

    // Update osu!alt held state
    if (Settings::OsuAltMode && mBtnIdx < 4u)
        sHeld[mBtnIdx] = mState;

    if (bufferElem > 0u)
    {
        mKeysPerSecond -= static_cast<float>(bufferElem);
        bufferElem = 0u;
        --statKeysPerSecond;
        assert(bufferElem == 0u);
    }
    if (!prevState && mState)
    {
        ++mKeysPerSecond;
        ++bufferElem;
        ++statKeysPerSecond;
        if (statKeysPerSecond > statMaxKeysPerSecond)
            statMaxKeysPerSecond = Settings::MaxKPS = statKeysPerSecond;

        const auto amtToAdd = 1u * Settings::ButtonPressMultiplier;
        mTotal += amtToAdd;
        statTotal += amtToAdd;
        Settings::Total += amtToAdd;
        Settings::KeysTotal[mBtnIdx] += amtToAdd;

        // Update osu!alt group tracking on new press
        if (Settings::OsuAltMode && mBtnIdx < 4u)
        {
            const int group = static_cast<int>(mBtnIdx % 2u); // 0 = left (0,2), 1 = right (1,3)
            // Compute color BEFORE updating sLastGroup so skip detection sees prior state
            const unsigned partner = mBtnIdx ^ 2u;
            if (partner < 4u && sHeld[partner])
                mPressAltColor = Settings::OsuAltLockColor;
            else if (sLastGroup == group)
                mPressAltColor = Settings::OsuAltSkipColor;
            else
                mPressAltColor = Settings::OsuAltNormalColor;
            sLastGroup = group;
        }
    }

    mPrevKpsBuffer[mPrevKpsBufferIndex] = mKeysPerSecond;
	accumulateBeatsPerMinute();
}

bool LogButton::isButtonPressed() const
{
    return mState;
}

void LogButton::moveIndex()
{
    if (++mBufferIndex == 60)
        mBufferIndex = 0;
    if (++mPrevKpsBufferIndex == 7)
        mPrevKpsBufferIndex = 0;

    statBeatsPerMinute = 0.f;
}

void LogButton::accumulateBeatsPerMinute()
{
	if (mBufferIndex != mLastAccumulateBpmBufferIndex)
	{
		statBeatsPerMinute += getLocalBeatsPerMinute();
		mLastAccumulateBpmBufferIndex = mBufferIndex;
	}
}

void LogButton::reset()
{
    mKeysPerSecond = statKeysPerSecond = statBeatsPerMinute = statMaxKeysPerSecond = Settings::MaxKPS = 0.f;
    mTotal = statTotal = Settings::Total = Settings::KeysTotal[mBtnIdx] = 0u;
    for (auto &elem : mBuffer)
        elem = 0;
    for (auto &elem : mPrevKpsBuffer)
        elem = 0;
}

float LogButton::getKeysPerSecond()
{
    return statKeysPerSecond;
}

float LogButton::getMaxKeysPerSecond()
{
    return statMaxKeysPerSecond;
}

unsigned LogButton::getTotal()
{
    return statTotal;
}

float LogButton::getBeatsPerMinute()
{
    return statBeatsPerMinute;
}

float LogButton::getLocalBeatsPerMinute() const
{
    float prevKpsSum = 0;
    for (const auto &elem : mPrevKpsBuffer)
        prevKpsSum += elem;

    // 15 = 60 (sec) / 4 (1/4 time signature for streams)
    return prevKpsSum / static_cast<float>(mPrevKpsBuffer.size()) * 15.f;
}

sf::Color LogButton::getAltColor() const
{
    return mPressAltColor;
}
