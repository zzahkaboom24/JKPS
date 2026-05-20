#pragma once

#include "LogKey.hpp"

#include <memory>
#include <array>
#include <SFML/Graphics/Color.hpp>


class LogButton
{
    public:
        LogButton(const unsigned idx, LogKey &key);

        void processRealtimeInput();

        static void moveIndex();
        bool isButtonPressed() const;

		void accumulateBeatsPerMinute();
        void reset();

        static float getKeysPerSecond();
        static float getMaxKeysPerSecond();
        static unsigned getTotal();
        static float getBeatsPerMinute();

        // Returns the color for the next press bar under osu!alt mode.
        // Must be called BEFORE processRealtimeInput() records the new press.
        sf::Color getAltColor() const;
    

    protected:
        float getLocalBeatsPerMinute() const;


    public:
        static float statKeysPerSecond;
        static float statMaxKeysPerSecond;
        static unsigned statTotal;
        static float statBeatsPerMinute;


    protected:
        LogKey mKey;
        float mKeysPerSecond;
        unsigned mTotal;


    private:
        const unsigned mBtnIdx;
        bool mState;
        sf::Color mPressAltColor;

        // Calculation related
        std::array<unsigned, 60lu> mBuffer;
        std::array<float, 7lu> mPrevKpsBuffer;
		unsigned mLastAccumulateBpmBufferIndex;
        static unsigned mBufferIndex;
        static unsigned mPrevKpsBufferIndex;
    
};