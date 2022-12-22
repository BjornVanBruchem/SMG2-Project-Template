	#pragma once

	#include "syati.h"
    #include "Game/Screen/CountUpPaneRumbler.h"
    
    class RedCoinCounter : public LayoutActor {
    public:
        RedCoinCounter(const char* pName);
        
        virtual void init(const JMapInfoIter& rIter);
        virtual void control();
        void appearIfHidden();
        void updateCounter(s32 count, bool hasAllCoins);
        void updateStarIndicator(s32 starID, s32 iconID);
    
        CountUpPaneRumbler* mPaneRumbler;
        bool mIsValidAppear;
    };
    
    class RedCoinCounterPlayer : public LayoutActor {
    public:
        RedCoinCounterPlayer(const char* pName);
        
        virtual void init(const JMapInfoIter& rIter);
        virtual void control();
        void calcScreenPos(TVec3f pos, f32 heightAdd);
        void updateCounter(s32 count);
    };