#ifdef SMSS

/*
*/
#include "pt/MapObj/BlueCoinSystem/BlueCoin.h"
#include "pt/MapObj/BlueCoinSystem/BlueCoinLayouts.h"
#include "Game/MapObj/CoinHolder.h"

BlueCoin::BlueCoin(const char* pName) : Coin(pName) {
    mID = 0;
    mLaunchVelocity = 250.0f;
    mUseConnection = false;

    MR::createCoinRotater();
    MR::createCoinHolder();
    MR::addToCoinHolder(this, this);
}

void BlueCoin::init(const JMapInfoIter& rIter) {
    MR::getJMapInfoArg0NoInit(rIter, &mID);
    MR::getJMapInfoArg1NoInit(rIter, &mLaunchVelocity);
    MR::getJMapInfoArg2NoInit(rIter, &mUseConnection);
    
    MR::processInitFunction(this, rIter, BlueCoinUtil::isBlueCoinGotCurrentFile(mID) ? "BlueCoinClear" : "BlueCoin", false);
    initEffectKeeper(2, "Coin", 0);
    MR::calcGravity(this);

    initNerve(&NrvCoin::CoinNrvFix::sInstance, 0);

    initHitSensor(1);
    MR::addHitSensor(this, "BlueCoin", 0x4A, 4, 55.0f, TVec3f(0.0f, 70.0f, 0.0f));
    
    MR::initShadowVolumeCylinder(this, 50.0f);
    MR::setShadowDropPositionPtr(this, 0, &mTranslation);

    mConnector = new MapObjConnector(this);

    mFlashingCtrl = new FlashingCtrl(this, 1);

    if (mUseConnection)
        MR::invalidateClipping(this);
    else
        MR::setClippingFarMax(this);

    makeActorAppeared();

    // Can't use ActorInfo for this one...
    MR::useStageSwitchSyncAppear(this, rIter);
}

void BlueCoin::initAfterPlacement() {
    if (MR::isValidSwitchB(this)) {
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
    }
}

void BlueCoin::control() {
    if (MR::isOnSwitchB(this) && MR::isHiddenModel(this))
        appearAndMove();
}

void BlueCoin::calcAndSetBaseMtx() {
    if (mUseConnection) {
        mConnector->connect();
        mConnector->attachToUnder();
    }

    Coin::calcAndSetBaseMtx();
}

bool BlueCoin::receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReciver) {
    if (MR::isMsgItemGet(msg)) {
        collect();
        return true;
    }

    return false;
}

void BlueCoin::appearAndMove() {
    // I need a better way to calculate the gravity
    TVec3f coinVelocity = TVec3f(0.0f, mLaunchVelocity / 10.0f, 0.0f);
    coinVelocity.scale(coinVelocity.y, -mGravity);

    MR::startSystemSE("SE_SY_PURPLE_COIN_APPEAR", -1, -1);
    
    appearMove(mTranslation, coinVelocity, 0x7FFFFFFF, 60);
}

void BlueCoin::collect() {
    MR::startSystemSE("SE_SY_PURPLE_COIN", -1, -1);
    MR::emitEffect(this, BlueCoinUtil::isBlueCoinGotCurrentFile(mID) ? "BlueCoinClearGet" : "BlueCoinGet");  
    
    if (MR::isValidSwitchA(this))
        MR::onSwitchA(this);

    if (!BlueCoinUtil::isBlueCoinGotCurrentFile(mID)) {
        BlueCoinUtil::setBlueCoinGotCurrentFile(mID);
        ((BlueCoinCounter*)MR::getGameSceneLayoutHolder()->mCounterLayoutController->mPTDBlueCoinCounter)->startCountUp();
    }

    GameSequenceFunction::getPlayResultInStageHolder()->addCoinNum(1);

    if (MR::isGalaxyDarkCometAppearInCurrentStage())
        MR::incPlayerLife(1);

    makeActorDead();
}

void appearCustomCoinOnDarkComet(LiveActor* pActor) {
    OSReport("%s\n", pActor->mName);
    if (!MR::isEqualString(pActor->mName, "RedCoin") || !MR::isEqualString(pActor->mName, "BlueCoin"))
        pActor->makeActorDead();
}

//kmCall(0x8028C2EC, appearCustomCoinOnDarkComet);
#endif