#include "syati.h"
#include "Game/Util.h"
#include "Game/System/Misc/GameSceneLayoutHolder.h"
#include "Game/Screen/LayoutActor.h"
#include "Game/System/Misc/TalkMessageCtrl.h"
#include "pt/Util/ActorUtil.h"

/*
* Authors: Aurum
*/
namespace pt {
	/*
	* Error Message Fallback
	*
	* Missing text will usualy crash the game. This small patch will display the text label whose message data could not be
	* located instead of nothing. This tells developers what text is missing.
	*/
	static wchar_t cErrorMessageBuffer[128]; // Buffer for missing text label. Last wchar should always be NULL.

	const wchar_t* getErrorMessage(const char *pLabel) {
		asm("mr %0, r31" : "=r" (pLabel));

		size_t len = strlen(pLabel) + 1;

		if (len > 127) {
			len = 127;
		}

		mbstowcs(cErrorMessageBuffer, pLabel, len);

		return cErrorMessageBuffer;
	}

	kmCall(0x800413F0, getErrorMessage); // MR::getGameMessageDirect will return the error message instead of NULL

	/*
	* Green Launch Star
	*
	* Unsurprisingly, all the BTP and BRK frames for the green Launch Star is still found inside SuperSpinDriver.arc. Here,
	* we hijack calls to initColor in order to check for the green color first. If the Launch Star's color is set to green,
	* we apply its animation frames. Otherwise, we call initColor to set up the other colors.
	*/
	void initSuperSpinDriverGreenColor(SuperSpinDriver *pActor) {
		if (pActor->mColor == SUPER_SPIN_DRIVER_GREEN) {
			MR::startBtpAndSetFrameAndStop(pActor, "SuperSpinDriver", 1.0f);
			MR::startBrk(pActor, "Green");

			pActor->mSpinDriverPathDrawer->mColor = 0; 
		}
		else {
			pActor->initColor();
		}
	}

	kmCall(0x8031E29C, initSuperSpinDriverGreenColor); // redirect initColor in init

	/*
	* The Green Launch Star is coded to load a model from SuperSpinDriverEmpty.arc. This was used for the transparent model
	* in SMG1 to mark its position before all green stars are collected. However, we have no use of this property in SMG2,
	* so we can safely disable this here. This also improves memory usage since this model would be loaded at all times.
	*/
	kmWrite32(0x8031E2A4, 0x60000000); // NOP call to initEmptyModel.


	///*
	//* Shell-casting Magikoopa
	//*/
	//void initNewKameck(Kameck *pActor, const JMapInfoIter &rIter) {
	//	pActor->initJMapParam(rIter);
	//
	//	if (MR::isValidInfo(rIter)) {
	//		if (MR::isObjectName(rIter, "KameckJetTurtle")) {
	//			pActor->mBeamType = KAMECK_BEAM_TURTLE;
	//		}
	//	}
	//}
	//
	//kmCall(0x801A49D4, initNewKameck);
	//
	///*
	//* While carrying over stuff from the first game, they forgot to update parts of the KameckTurtle actor. Therefore,
	//* it will crash the game and cause various other problems. First of all, it tries to load the animation from SMG1,
	//* which does not exist anymore (Koura.brk was renamed to Color.brk). Also, Mario won't try to pull in the shell
	//* due to the shell's actor name being wrong. For some reason it expects a specific actor name...
	//* Lastly, the actor should be dead by default, but they made it appear nevertheless.
	//*/
	//void initFixKameckTurtle(LiveActor *pActor) {
	//	pActor->mName = "�J���b�N�r�[���p�J��";
	//	MR::startBrk(pActor, "Color");
	//}
	//
	//kmCall(0x801A8CFC, initFixKameckTurtle); // redirect BRK assignment to initialization fix
	//kmWrite32(0x801A8DD0, 0x818C0038);       // Call makeActorDead instead of makeActorAppeared


	/*
	* KeySwitch fix
	*
	* If a KeySwitch is constructed by Teresa or KuriboChief, it will crash the game due to an oversight by the developers.
	* This feature works perfectly fine in SMG1., however, SMG2 introduced a new Obj_arg that does not check if the given
	* JMapInfoIter is valid before attempting to read the Obj_arg value. This will cause the game to access invalid data
	* and thus causing a crash.
	*/
	void initKeySwitchSafeGetShadowDropLength(const JMapInfoIter &rIter, f32 *pDest) {
		if (MR::isValidInfo(rIter)) {
			MR::getJMapInfoArg0NoInit(rIter, pDest);
		}
	}

	kmCall(0x802BDE80, initKeySwitchSafeGetShadowDropLength); // overwrite call to getJMapInfoArg0NoInit


	/*
	* QuakeEffectGenerator fix
	*
	* QuakeEffectGenerator never plays the earthquake sound as its sound object is never initialized. This is likely an
	* oversight from when the SMG2 developers upgraded SMG1's sound system since sounds are loaded slightly different in
	* SMG1.
	*/
	void initQuakeEffectGeneratorSound(LiveActor *pActor) {
		MR::invalidateClipping(pActor);
		pActor->initSound(1, "QuakeEffectGenerator", &pActor->mTranslation, TVec3f(0.0f, 0.0f, 0.0f));
	}

	kmCall(0x8026360C, initQuakeEffectGeneratorSound); // redirection hook

	/*
	* Debugging feature: displaying the file name on the "File isn't exist" error.
	*
	* When the game attempts to load a file into memory, it runs MR::isFileExist to check for the file, and if the file it's checking
	* for doesn't exist, it calls OSFatal,  crashing the game. It also prints "File isn't exist" to the log.
	*
	* Here, the MR::isFileExist call is replaced with a call to this new function, that prints the file name with the error, if the checked file is missing.
	*
	* This is useful for debugging certain things!
	*/

	void printFileNameIfMissing(const char* fileName) {
		if (!MR::isFileExist(fileName, 0))
			OSPanic("FileRipper.cpp", 118, "File \"%s\" isn't exist.", fileName);
	}

	kmCall(0x804B1FE0, printFileNameIfMissing);

	/*
	* Mini Patch: Swimming Death Area
	* 
	* This patch is really useless but I thought it would be nice to include.
	* For example, this could be used to make instant death water/lava.
	*/

	void DeathAreaExtensions(DeathArea* area) {
		MR::getGameSceneLayoutHolder()->mMarioSubMeter->mAirMeter->mLayoutActorFlag.mIsHidden = area->isInVolume(*MR::getPlayerPos());

		bool checkForSwimming;

		if (area->mObjArg1 == -1)
			checkForSwimming = false;
		else
			checkForSwimming = true;

		if (area->isInVolume(*MR::getPlayerPos()) && checkForSwimming ? MR::isPlayerSwimming() : true) 
			MarioAccess::forceKill(checkForSwimming ? 3 : 0, 0);
	}

	kmCall(0x8007401C, DeathAreaExtensions);
	kmWrite32(0x8007402C, 0x60000000);


	bool isInDeathFix(const char* name, const TVec3f& pos) {
		if (MR::isInAreaObj(name, pos))
			if (MR::getAreaObj(name, pos)->mObjArg1 == -1 || MR::isInWater(pos))
				return true;

		return false;
	}

	kmBranch(0x8004AC1C, isInDeathFix);

	
	// Fix for Yoshi since he was coded differently.
	kmWrite32(0x804129EC, 0x7FE3FB78); // mr r3, r31
	kmWrite32(0x804129F4, 0x4BBFCABD); // Replace call to MR::isInAreaObj with MR::isInDeath.

	/*
	* Mini Patch: Custom HipDropSwitch colors
	* 
	* A fun but useless patch suggested by Alex SMG.
	*/

	void customHipDropSwitchColors(LiveActor* actor, const JMapInfoIter& iter) {
		MR::needStageSwitchWriteA(actor, iter);

		s32 frame = 0;
		MR::getJMapInfoArg1NoInit(iter, &frame);
		MR::startBtpAndSetFrameAndStop(actor, "ButtonColor", frame);
	}
	
	kmCall(0x802AF524, customHipDropSwitchColors);

	/*
	* Mini Patch: Ocean Sphere Texture Patch
	* 
	* The TearDropGalaxy and SkullSharkGalaxy checks for setting the texture are in SMG2.
	* Here we change it to read Obj_arg0, so the second texture can be used in custom galaxies.
	*/

	s32 OceanSphereTexturePatch(const JMapInfoIter& iter) {
		s32 arg = 0;
		MR::getJMapInfoArg0NoInit(iter, &arg);
		return arg;
	}

	kmWrite32(0x8025CE34, 0x7FC3F378); // mr r3, r30
	kmCall(0x8025CE38, OceanSphereTexturePatch); // Hook

	/*
	* Mini Patch: Yes/No Dialogue Extensions
	* 
	* Adds the ability to create custom Yes/No MSBF dialogue options.
	* 
	* Be sure to add text entries to /SystemMessage.arc/Select.msbt.
	* The format is "Select_Name_Yes", and the same thing for No.
	* 
	* Three new custom entries are added, so the source of PTD
	* doesn't need to be edited for custom entries.
	* 
	* Knowledge of MSBF is required for this to be of any use in game.
	*/

	const char* YesNoDialogueExtensions(const TalkMessageCtrl* msg) {
		s32 selectTxt;
		msg->mTalkNodeCtrl->getNextNodeBranch();
		asm("lhz %0, 0x8(r3)" : "=r" (selectTxt)); // Temporary workaround until we figure out what class this is in.

		char* str = new char[7];
		sprintf(str, "New%d", selectTxt - 18);

		return selectTxt < 18 ? msg->getBranchID() : str;
	}

	kmCall(0x80379A84, YesNoDialogueExtensions);

	//void sus(LiveActor* actor, const JMapInfoIter& iter) {
	//	MR::useStageSwitchWriteA(actor, iter);
	//	MR::declareStarPiece(actor, 0x18);
	//	OSReport("switch %s\n", actor->mName);
	//}
	//
	//kmWrite32(0x800DAABC, 0x60000000);
	////kmWrite32(0x800DAAC4, 0x7F63DB78);
	////kmCall(0x800DAAC8, sus);

	//void sus2(const NameObj* obj) {
	//	if (MR::isValidSwitchA(obj))
	//		MR::onSwitchA(obj);
	//}
}