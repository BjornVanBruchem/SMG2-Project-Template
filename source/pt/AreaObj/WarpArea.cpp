#include "pt/AreaObj/WarpArea.h"
#include "pt/Extras/WarpAreaSystem.h"
#include "Game/Util.h"
using namespace pt;

/*
* Created by Evanbowl with help from Aurum, AwesomeTMC, Galaxy Master, and Zyphro.
* 
* An area that takes the player to a specified General Position with the name "WarpAreaDestPos".
* Can also take the player to galaxies.
*
* This is PT Debug exclusive due to a similar feature being in Galaxy Level Engine.
*/

WarpArea::WarpArea(const char* pName) : AreaObj(pName) {
	mElapsed = 0;
	mCanWarp = false;
	mPos.set(0.0f, 0.0f, 0.0f);
	mStageTable = new WarpAreaStageTable(true); // BCSV table for going to galaxies.
	mErrorLayout = new ErrorLayout(); // Error layout used for displaying messages on screen if a WarpArea function fails.
}

void WarpArea::init(const JMapInfoIter& rIter) {
	AreaObj::init(rIter);
	MR::connectToSceneAreaObj(this);

	if (mFadeCloseTime < 0)
		mFadeCloseTime = 45;

	mErrorLayout->initWithoutIter();
}

void WarpArea::movement() {
	if (isInVolume(*MR::getPlayerPos()))
		mCanWarp = true;
	
		if (mCanWarp) {
		mElapsed++;
	
		// Phase 1: Start a circle wipe and lock player control
		if (mElapsed == 10) {
			MR::offPlayerControl();
			MR::disableStarPointerShootStarPiece();
			mStageTable->selectWipeClose(mFadeCloseType, mFadeCloseTime);
		
		if (mPosID < 0) { // If the player is warping to a galaxy, fade out all music.
			MR::stopSubBGM(45);
			MR::stopStageBGM(45);
		}
	}

		// Phase 2: Set the player state to Wait to prevent animation issues and then warp Mario to the specified GeneralPos. Or, go to a galaxy.
		if (mElapsed == mFadeCloseTime + 50) {
			if (mPosID >= 0) {
				char WarpAreaDestPos[0xF];
				sprintf(WarpAreaDestPos, "WarpAreaPos%03d", mPosID); // Combines WarpAreaDestPos + the value of mObjArg0.
				MR::setPlayerPosAndWait(WarpAreaDestPos); // Teleports the player to WarpAreaDestPos[mObjArg0].
			
				if (!MR::findNamePos(WarpAreaDestPos, &mPos, &mPos)) { // Check if the specified general position exists
					mErrorLayout->printf(mPrintErrors, "Failed. WarpArea position %s isn't exist.", WarpAreaDestPos); //If not, then print a message on screen.
					NameObjFunction::requestMovementOff(this);
				}
			}
			else {
				mStageTable->readTable(mIndex, mPrintErrors); // If no general position is specified, set up a galaxy transition by reading the BCSV.
				NameObjFunction::requestMovementOff(this);
			}
		}
	
		// Phase 3: Open the wipe and restore player control.
		if (mElapsed == mFadeCloseTime + 90) {
			if (mPosID >= 0) {
				mStageTable->selectWipeOpen(mFadeOpenType, mFadeOpenTime);
				MR::onPlayerControl(1);
				mElapsed = 0;
				mCanWarp = false;
			}
			else { // This will only appear if a galaxy transition fails, specifically if the specified index is not in the bcsv
				MR::openSystemWipeCircle(45);
				MR::onPlayerControl(1);
				mErrorLayout->printf(mPrintErrors, "Failed. BCSV Index %d isn't exist.", mPosID);
				NameObjFunction::requestMovementOff(this);
				}
			}
		}
	}

const char* WarpArea::getManagerName() const {
	return "BindEnd";
}