#ifdef SMG63
#include "pt/AreaObj/MarioDisappointmentArea.h"
#include "syati.h"

MarioDisappointmentArea::MarioDisappointmentArea(const char* pName) : AreaObj(pName) {}

void MarioDisappointmentArea::init(const JMapInfoIter& rIter) {
    AreaObj::init(rIter);
	MR::connectToSceneAreaObj(this);
}

void MarioDisappointmentArea::movement() {
    if (isInVolume(*MR::getPlayerPos())) {
        if (mObjArg0 > -1) {
            MR::disableStarPointerShootStarPiece();
        }
        else {
            MarioAccess::getPlayerActor()->_F9C = false;
            MarioAccess::getPlayerActor()->_F9A = 1;
        }
    }
}

const char* MarioDisappointmentArea::getManagerName() const {
    return "SwitchArea";
}
#endif