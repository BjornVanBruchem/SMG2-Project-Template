#if defined USEBLUECOIN && !defined SM64BLUECOIN && defined SMSS || defined ALL
#include "pt/MapObj/BlueCoinSystem/BlueCoinBoard.h"

#ifdef ALL
    #define STAGE_NAME "TemplateTestGalaxy"
#elif SMSS
    #define STAGE_NAME "BlueCoinBonusGalaxy"
#endif

/*
    Super Mario Starshine: Blue Coin Board

    This layout handles the purchasing and entering of the Blue Coin bonus stages.

    Blue Coin Flags were only ever implemented since I planned very far ahead.
    This is the only reason why they ever existed.

    Made for Starshine only. Do not ask me for this in your mods.

    First Concepted in 5/2023
    Started 9/24/2023
    Finished 10/1/23
    Revealed 

    Made by Evanbowl
    
    I thank SPG64, Lord Giganticus, and Xandog for very helpful feedback.
*/

BlueCoinSign::BlueCoinSign(const char* pName) : NPCActor(pName) {
    pBoard = 0;
}

void BlueCoinSign::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps = NPCActorCaps("BlueCoinSign");
    caps.setDefault();
    caps.mInitLightCtrl = 0;
    caps.mInitYoshiLockOnTarget = 0;
    caps.mWaitNerve = &NrvBlueCoinSign::NrvWait::sInstance;

    initialize(rIter, caps, 0, 0, 0);

    mTalkCtrl->_50 = 0;

    MR::registerEventFunc(mTalkCtrl, TalkMessageFunc(this, &eventFunc));

    pBoard = new BlueCoinBoard("BlueCoinBoard");
    pBoard->initWithoutIter();
    makeActorAppeared();
}

bool BlueCoinSign::eventFunc(u32 yes) {
    if (isNerve(&NrvBlueCoinSign::NrvOpen::sInstance)) {
        if (MR::isDead(pBoard)) {
            if (pBoard->mHasSpentBlueCoins)
                BlueCoinUtil::startCounterCountUp();

            popNerve();
            return true;
        }
        else
            return false;
    }
    else {
        if (MR::isExistSceneObj(SCENE_OBJ_PAUSE_BLUR))
            MR::requestMovementOn(MR::getSceneObjHolder()->getObj(SCENE_OBJ_PAUSE_BLUR));

        pushNerve(&NrvBlueCoinSign::NrvOpen::sInstance);
        return false;
    }

    return false;
}

void BlueCoinSign::exeWait() {
    MR::tryTalkNearPlayer(mTalkCtrl);
}

void BlueCoinSign::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startSystemSE("SE_SY_STAR_RESULT_PANEL_OPEN", -1, -1);
        pBoard->appear();
    }
}

namespace NrvBlueCoinSign {
	void NrvWait::execute(Spine* pSpine) const {
        ((BlueCoinSign*)pSpine->mExecutor)->exeWait();
	}

	void NrvOpen::execute(Spine* pSpine) const {
        ((BlueCoinSign*)pSpine->mExecutor)->exeOpen();
	}

	NrvWait(NrvWait::sInstance);
    NrvOpen(NrvOpen::sInstance);
}


BlueCoinBoard::BlueCoinBoard(const char* pName) : LayoutActor(pName, 0) {
    for (s32 i = 0; i < 8; i++) {
        mButtons[i] = 0;
        mButtonFollowPositions[i] = TVec2f(0.0f, 0.0f);
    }

    mSelectedButton = -1;
    mSysInfoWindowSelect = 0;
    mBlueCoinPaneRumbler = 0;
    mBlueCoinNumToDisplay = 0;
    mBackButton = 0;
    mHasSpentBlueCoins = 0;
}

void BlueCoinBoard::init(const JMapInfoIter& rIter) {
    initLayoutManager("BlueCoinBoard", 2);
    MR::registerDemoSimpleCastAll(this);
    initNerve(&NrvBlueCoinBoard::NrvHide::sInstance);
    MR::connectToScene(this, 0xE, 0xD, -1, 0x48);

    mBackButton = new BackButtonCancelB("BackButtonForBlueCoin", 0);
    MR::connectToScene(mBackButton, 0xE, 0xD, -1, 0x48);
    mBackButton->initWithoutIter();

    mSysInfoWindowSelect = MR::createSysInfoWindow();
    MR::connectToScene(mSysInfoWindowSelect, 0xE, 0xD, -1, 0x48);

    mSysInfoWindowBox = MR::createSysInfoWindowMiniExecuteWithChildren();
    MR::connectToScene(mSysInfoWindowBox, 0xE, 0xD, -1, 0x48);

    mBlueCoinPaneRumbler = new CountUpPaneRumbler(this, "CounterBlueCoin");
    mBlueCoinPaneRumbler->mRumbleCalculator->mRumbleStrength = 8.0f;
    mBlueCoinPaneRumbler->reset();

    MR::createAndAddPaneCtrl(this, "Misc", 1);

    MR::createAndAddPaneCtrl(this, "StarCounter", 1);
    MR::createAndAddPaneCtrl(this, "BlueCoinCounter", 1);
    MR::setFollowPos(&mBlueCoinCounterFollowPos, this, "BlueCoinCounter");
    MR::copyPaneTrans(&mBlueCoinCounterFollowPos, this, BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true) >= 100 ? "BlueCoinPos100" : "BlueCoinPos10");

    for (s32 i = 0; i < 8; i++) {
        snprintf(mBoxButtonName[i], 12, "BoxButton%d", i);
        snprintf(mButtonName[i], 13, "CoinButton%d", i);
        snprintf(mButtonTxtName[i], 13, "Button%dText", i);
        snprintf(mFollowPosName[i], 9, "Button%d", i);
        snprintf(mCopyPosName[i], 12, "Button%dPos", i);

        MR::createAndAddPaneCtrl(this, mFollowPosName[i], 1);

        mButtons[i] = new ButtonPaneController(this, mButtonName[i], mBoxButtonName[i], 0, 1);
        mButtons[i]->_26 = false;

        MR::setFollowPos(&mButtonFollowPositions[i], this, mFollowPosName[i]);
        MR::setFollowTypeReplace(this, mFollowPosName[i]);
    }
}

void BlueCoinBoard::appear() {
    setNerve(&NrvBlueCoinBoard::NrvAppear::sInstance);
    LayoutActor::appear();
}

void BlueCoinBoard::exeAppear() {
    if (MR::isFirstStep(this)) {
        for (s32 i = 0; i < 8; i++) {
            const char* label = "BoardButton_Locked";

            if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(i)) {
                label = "BoardButton_UnlockedUncleared";

                if (MR::makeGalaxyStatusAccessor(STAGE_NAME).hasPowerStar(i+1))
                    label = "BoardButton_UnlockedCleared";
            }

            MR::setTextBoxGameMessageRecursive(this, mButtonTxtName[i], label);
            MR::setTextBoxArgNumberRecursive(this, mButtonTxtName[i], i+1, 0);
            mButtons[i]->appear();
        }

        mHasSpentBlueCoins = false;

        MR::requestMovementOn(mBackButton);
        MR::requestMovementOn(mSysInfoWindowSelect);
        MR::requestMovementOn(mSysInfoWindowBox);

        if (MR::isExistSceneObj(SCENE_OBJ_PAUSE_BLUR))
            ((PauseBlur*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_PAUSE_BLUR))->_30+=1;

        MR::startAnim(this, "Appear", 1);
        MR::startStarPointerModeChooseYesNo(this);

        mBlueCoinNumToDisplay = BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true);

        MR::setTextBoxNumberRecursive(this, "CounterBlueCoin", mBlueCoinNumToDisplay);
        MR::setTextBoxNumberRecursive(this, "CounterStar", MR::makeGalaxyStatusAccessor(STAGE_NAME).getPowerStarNumOwnedTotal());

        MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_NoSelection");
        MR::setTextBoxGameMessageRecursive(this, "TextTitle", "Board_Title");

        mBackButton->appear();

        setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

void BlueCoinBoard::control() {
    for (s32 i = 0; i < 8; i++) {
        MR::copyPaneTrans(&mButtonFollowPositions[i], this, mCopyPosName[i]);
        mButtons[i]->update();
    }
    
    mBlueCoinPaneRumbler->update();
}

void BlueCoinBoard::exeSelecting() {
    if (MR::isFirstStep(this))
        MR::requestMovementOn(mBackButton);

    mSelectedButton = -1;
    s32 pointedButton = -1;

    for (s32 i = 0; i < 8; i++) {
        if (mButtons[i]->trySelect())
            mSelectedButton = i;

        if (mButtons[i]->isPointing())
            pointedButton = i;

        if (mButtons[i]->isPointingTrigger())
            MR::startSystemSE("SE_SY_SELECT_PAUSE_ITEM", -1, -1);
    }

    if (pointedButton > -1) {
        const char* label = "WinBase_Locked";

        if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(pointedButton)) {
            label = "WinBase_UnlockedUncleared";

            if (MR::makeGalaxyStatusAccessor(STAGE_NAME).hasPowerStar(pointedButton+1))
                label = "WinBase_UnlockedCleared";
        }

        MR::setTextBoxGameMessageRecursive(this, "TextWinBase", label);
        MR::setTextBoxArgNumberRecursive(this, "TextWinBase", pointedButton+1, 0);
    }
    else
        MR::setTextBoxGameMessageRecursive(this, "TextWinBase", "WinBase_NoSelection");

    if (mSelectedButton > -1) {
        pointedButton = mSelectedButton;
        setNerve(&NrvBlueCoinBoard::NrvSelected::sInstance);
    }

    if (mBackButton->_30)
        setNerve(&NrvBlueCoinBoard::NrvDisappear::sInstance);
}

void BlueCoinBoard::exeDisappear() { 
    if (MR::isStep(this, 30)) {
        for (s32 i = 0; i < 8; i++) {
            mButtons[i]->disappear();
        }

        if (MR::isExistSceneObj(SCENE_OBJ_PAUSE_BLUR))
            ((PauseBlur*)MR::getSceneObjHolder()->getObj(SCENE_OBJ_PAUSE_BLUR))->_30-=1;
        
        MR::startAnim(this, "End", 0);
    }

    if (MR::isStep(this, 50)) {
        MR::endStarPointerMode(this);
        kill();
    }
}

void BlueCoinBoard::exeSelected() {
    if (MR::isFirstStep(this))
        MR::requestMovementOff(mBackButton);

    if (mButtons[mSelectedButton]->isTimingForSelectedSe())
        MR::startSystemSE("SE_SY_PAUSE_OFF", -1, -1);

    if (MR::isStep(this, 25)) {
        for (s32 i = 0; i < 8; i++) {
            mButtons[i]->forceToWait();
        }

        if (BlueCoinUtil::isOnBlueCoinFlagCurrentFile(mSelectedButton))
            setNerve(&NrvBlueCoinBoard::NrvConfirmPlayStage::sInstance);
        else
            setNerve(&NrvBlueCoinBoard::NrvConfirmUnlock::sInstance);
    }
}

void BlueCoinBoard::exeConfirmUnlock() {
    if (MR::isFirstStep(this)) {
        MR::requestMovementOn(mSysInfoWindowSelect);
        mSysInfoWindowSelect->appear("BoardInfoWindow_ConfirmUnlockStage", SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);
        MR::setTextBoxArgNumberRecursive(mSysInfoWindowSelect, mSysInfoWindowSelect->_3C, mSelectedButton+1, 0);
    }

    if (MR::isDead(mSysInfoWindowSelect)) {
        if (mSysInfoWindowSelect->isSelectedYes()) {
            if (BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true) >= 30)
                setNerve(&NrvBlueCoinBoard::NrvCountDownBlueCoin::sInstance);
            else
                setNerve(&NrvBlueCoinBoard::NrvNotEnoughBlueCoins::sInstance);
        }
        else
            setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

void BlueCoinBoard::exeCountDownBlueCoin() {
    if (MR::isFirstStep(this)) {
        BlueCoinUtil::spendBlueCoinCurrentFile(30);
        BlueCoinUtil::setOnBlueCoinFlagCurrentFile(mSelectedButton);
    }

    if (mBlueCoinNumToDisplay > BlueCoinUtil::getTotalBlueCoinNumCurrentFile(true)) {
        if (getNerveStep() % 2 == 0)
            MR::startSystemSE("SE_SY_PURPLE_COIN", -1, -1);

        mBlueCoinNumToDisplay--;
    }
    else {
        MR::startPaneAnim(this, "CounterBlueCoin", "Flash", 0);
        mBlueCoinPaneRumbler->start();
        mHasSpentBlueCoins = true;
        setNerve(&NrvBlueCoinBoard::NrvChangeButtonText::sInstance);
    }

    MR::setTextBoxNumberRecursive(this, "CounterBlueCoin", mBlueCoinNumToDisplay);  
    MR::copyPaneTrans(&mBlueCoinCounterFollowPos, this, mBlueCoinNumToDisplay >= 100 ? "BlueCoinPos100" : "BlueCoinPos10");
}

void BlueCoinBoard::exeNotEnoughBlueCoins() {
    if (MR::isFirstStep(this))
        mSysInfoWindowBox->appear("BoardInfoWindow_NotEnoughBlueCoins", SysInfoWindow::SysInfoType_0, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);

    if (MR::isDead(mSysInfoWindowBox))
        setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
}

void BlueCoinBoard::exeChangeButtonText() {
    if (MR::isStep(this, 20)) {
        MR::setTextBoxGameMessageRecursive(this, mButtonTxtName[mSelectedButton], "BoardButton_UnlockedUncleared");
        MR::setTextBoxArgNumberRecursive(this, mButtonTxtName[mSelectedButton], mSelectedButton+1, 0);
    }

    if (MR::isStep(this, 30))
        setNerve(&NrvBlueCoinBoard::NrvConfirmPlayStage::sInstance);
}

void BlueCoinBoard::exeConfirmPlayStage() {
    if (MR::isFirstStep(this)) {
        mSysInfoWindowSelect->appear("BoardInfoWindow_ConfirmPlayStage", SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);
        MR::setTextBoxArgNumberRecursive(mSysInfoWindowSelect, mSysInfoWindowSelect->_3C, mSelectedButton+1, 0);
    }

    if (MR::isDead(mSysInfoWindowSelect)) {
        if (mSysInfoWindowSelect->isSelectedYes()) {
            MR::startSystemWipeCircleWithCaptureScreen(0x5A);
            GameSequenceFunction::requestChangeScenarioSelect(STAGE_NAME);
    	    GameSequenceFunction::requestChangeStage(STAGE_NAME, mSelectedButton+1, mSelectedButton+1, JMapIdInfo(0, 0));
        }
        else
            setNerve(&NrvBlueCoinBoard::NrvSelecting::sInstance);
    }
}

namespace NrvBlueCoinBoard {
	void NrvAppear::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeAppear();
	}

	void NrvSelecting::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeSelecting();
	}

	void NrvDisappear::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeDisappear();
	}

	void NrvHide::execute(Spine* pSpine) const {}

    void NrvSelected::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeSelected();
    }

    void NrvConfirmUnlock::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeConfirmUnlock();
    }

    void NrvCountDownBlueCoin::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeCountDownBlueCoin();
    }

    void NrvNotEnoughBlueCoins::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeNotEnoughBlueCoins();
    }

    void NrvChangeButtonText::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeChangeButtonText();
    }

    void NrvConfirmPlayStage::execute(Spine* pSpine) const {
        ((BlueCoinBoard*)pSpine->mExecutor)->exeConfirmPlayStage();
    }

	NrvAppear(NrvAppear::sInstance);
    NrvSelecting(NrvSelecting::sInstance);
    NrvDisappear(NrvDisappear::sInstance);
    NrvHide(NrvHide::sInstance);
    NrvSelected(NrvSelected::sInstance);
    NrvConfirmUnlock(NrvConfirmUnlock::sInstance);
    NrvCountDownBlueCoin(NrvCountDownBlueCoin::sInstance);
    NrvNotEnoughBlueCoins(NrvNotEnoughBlueCoins::sInstance);
    NrvChangeButtonText(NrvChangeButtonText::sInstance);
    NrvConfirmPlayStage(NrvConfirmPlayStage::sInstance);
}
#endif