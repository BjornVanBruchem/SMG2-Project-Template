#if defined ALL || defined SMSS || defined SMG63

#include "syati.h"
#include "Game/Screen/PauseMenu.h"

kmWrite32(0x804712C0, 0x3860006C); // li r3, 0x6C

// init hook

#ifdef SMSS 
#define STAGE_CHECK MR::isStageMarioFaceShipOrWorldMap() || (MR::isEqualStageName("YosshiHomeGalaxy") && MR::getCurrentScenarioNo() == 1) || MR::isEqualStageName("PrisonGalaxy")
#else
#define STAGE_CHECK MR::isStageMarioFaceShipOrWorldMap() || MR::isEqualStageName("PeachCastleGalaxy")
#endif

void setButtonAnimNames(ButtonPaneController* pButton) {
    pButton->mAnimNameAppear = "ButtonAppear_restartbutton";
    pButton->mAnimNameDecide = "ButtonDecide_restartbutton";
    pButton->mAnimNameEnd = "ButtonEnd_restartbutton";
    pButton->mAnimNameSelectIn = "ButtonSelectIn_restartbutton";
    pButton->mAnimNameSelectOut = "ButtonSelectOut_restartbutton";
    pButton->mAnimNameWait = "ButtonWait_restartbutton";
}


void PauseMenuInitNewButton(PauseMenu* pPauseMenu, const Nerve* pNerve) { 
    pPauseMenu->mButtonNew = 0;
    pPauseMenu->mIsUsedNewButton = 0;
    pPauseMenu->mButtonNewFollowPos = TVec2f(0.0f, 0.0f);

    MR::createAndAddPaneCtrl(pPauseMenu, "NewButton", 1);

    if (!(STAGE_CHECK)) {
        pPauseMenu->mButtonNew = new ButtonPaneController(pPauseMenu, "NBackNew", "BoxButton4", 0, 1);
        pPauseMenu->mButtonNew->mFadeAfterSelect = false;
    
        MR::setTextBoxFormatRecursive(pPauseMenu, "Text4", L"Restart Stage");

        MR::setFollowPos(&pPauseMenu->mButtonNewFollowPos, pPauseMenu, "NewButton");
        MR::setFollowTypeReplace(pPauseMenu, "NewButton");

        setButtonAnimNames(pPauseMenu->mButtonTop);
        setButtonAnimNames(pPauseMenu->mButtonBottom);
        setButtonAnimNames(pPauseMenu->mButtonNew);
    }
    else
        MR::hidePaneRecursive(pPauseMenu, "NBackNew");

    pPauseMenu->initNerve(pNerve);
}

kmCall(0x8048702C, PauseMenuInitNewButton); // Call

void ButtonControl(TVec2f* pPos, PauseMenu* pPauseMenu, const char* pStr) {
    MR::copyPaneTrans(&pPauseMenu->mButtonTopFollowPos, pPauseMenu, pStr);

    if (pPauseMenu->mButtonNew) {
        MR::copyPaneTrans(&pPauseMenu->mButtonNewFollowPos, pPauseMenu, "NewButtonPos");
        pPauseMenu->mButtonNew->update();
    }
}

kmCall(0x8048727C, ButtonControl); // Call

void PauseMenuSetButtonPosition(PauseMenu* pPauseMenu, const char* pStr1, const char* pStr2, f32 frame, u32 u) {
    MR::startPaneAnimAndSetFrameAndStop(pPauseMenu, pStr1, pPauseMenu->mButtonNew ? "ButtonPosition_restartbutton" : pStr2, frame, u);
}

kmCall(0x804874D4, PauseMenuSetButtonPosition);

void ForceToWaitNewButton(PauseMenu* pPauseMenu) {
    pPauseMenu->mButtonTop->forceToWait();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->forceToWait();
}

kmWrite32(0x80487504, 0x60000000); // nop (Skip overwriting r3)
kmCall(0x80487508, ForceToWaitNewButton); // Call

void PauseMenuAppearNewButton(PauseMenu* pPauseMenu) {
    pPauseMenu->mButtonTop->appear();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->appear();
}

kmWrite32(0x80487560, 0x7FE3FB78); // mr r3, r31 (PauseMenu* into r3)
kmCall(0x80487564, PauseMenuAppearNewButton); // Call

bool PauseMenuIsNewButtonPointingTrigger(PauseMenu* pPauseMenu) {
    return (pPauseMenu->mButtonTop && pPauseMenu->mButtonTop->isPointingTrigger()) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isPointingTrigger());
}

kmWrite32(0x80487714, 0x7F63DB78); // mr r3, r27 (PauseMenu* into r3)
kmCall(0x80487720, PauseMenuIsNewButtonPointingTrigger);

bool IsNewButtonPressed(PauseMenu* pPauseMenu) {
    bool isPressed = false;

    if (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->trySelect()) {
        pPauseMenu->mIsUsedNewButton = false;
        isPressed = true;
    }

    if (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->trySelect()) {
        pPauseMenu->mIsUsedNewButton = true;
        isPressed = true;
    }

    return isPressed;
}

kmWrite32(0x804877B4, 0x7F63DB78); // mr r3, r27 (PauseMenu* into r3)
kmCall(0x804877C0, IsNewButtonPressed); // Call

void PauseMenuSetInfoWindowStr(PauseMenu* pPauseMenu, const char* pStr) {
    pPauseMenu->mSysInfoWindow->appear(pStr, SysInfoWindow::SysInfoType_2, SysInfoWindow::SysInfoTextPos_0, SysInfoWindow::SysInfoMessageType_1);

    if (pPauseMenu->mIsUsedNewButton) 
        MR::setTextBoxFormatRecursive(pPauseMenu->mSysInfoWindow, "TxtConfirm", L"Restart current stage?");
}

kmWrite32(0x80487C4C, 0x7FE3FB78); // mr r3, r31 (PauseMenu* into r4)
kmCall(0x80487C5C, PauseMenuSetInfoWindowStr); // Call

void DisappearNewButton(PauseMenu* pPauseMenu) {
    pPauseMenu->mButtonTop->disappear();

    if (pPauseMenu->mButtonNew)
        pPauseMenu->mButtonNew->disappear();
}

kmWrite32(0x80487B10, 0x7FC3F378); // mr r3, r30 (PauseMenu* into r3)
kmCall(0x80487B14, DisappearNewButton); // Call

bool IsNewButtonTimingForSelectedSE(PauseMenu* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->isTimingForSelectedSe()) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isTimingForSelectedSe());
}

kmWrite32(0x804879C8, 0x7FC3F378); // mr r3, r30
kmCall(0x804879CC, IsNewButtonTimingForSelectedSE); // Call

bool PauseMenuValidateButton(PauseMenu* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->_24) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->_24);
}

kmWrite32(0x804879B0, 0x7FC3F378); // mr r3, r30
kmCall(0x804879B4, PauseMenuValidateButton);
kmWrite32(0x804879B8, 0x2C030000); // cmpwi r3, 0
kmWrite32(0x804879BC, 0x41820080); // beq 0x80
kmWrite32(0x804879C0, 0x48000008); // b 0x8


bool IsNewButtonDecidedWait(PauseMenu* pPauseMenu) {
    return (pPauseMenu->mButtonBottom && pPauseMenu->mButtonBottom->isDecidedWait()) || (pPauseMenu->mButtonNew && pPauseMenu->mButtonNew->isDecidedWait());
}

kmWrite32(0x80487A00, 0x7FC3F378); // mr r3, r30 (PauseMenu* into r3)
kmCall(0x80487A04, IsNewButtonDecidedWait); // Call

kmWrite32(0x80487A3C, 0x807E0034); // lwz r3, 0x34(r30)

void DoNewButtonAction(PauseMenu* pPauseMenu, bool isValid) {
    if (!pPauseMenu->mIsUsedNewButton)
        isValid ? GameSequenceFunction::notifyToGameSequenceProgressToEndScene() : GameSequenceFunction::requestChangeStageWorldMap();
    else {
        GameSequenceFunction::requestChangeScenarioSelect(MR::getCurrentStageName());
		GameSequenceFunction::requestChangeStage(MR::getCurrentStageName(), MR::getCurrentScenarioNo(), MR::getCurrentSelectedScenarioNo(), JMapIdInfo(0, 0));
    }
}

kmWrite32(0x80487CA4, 0x7C641B78); // mr r4, r3 (isValid into r4)
kmWrite32(0x80487CA8, 0x7FE3FB78); // mr r3, r31 (PauseMenu* into r3)
kmCall(0x80487CAC, DoNewButtonAction); // Call
kmWrite32(0x80487CB0, 0x48000008); // b 0x8 (Skip useless instructions)
#endif