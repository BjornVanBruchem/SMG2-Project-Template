#include "pt/MapObj/RedCoinSystem/RedCoin.h"
#include "Game/MapObj/CoinHolder.h"

/*
* These actors are exclusive to PT Debug.
* RedCoin, RedCoinController
*
* A long awaited project that I finally decided to scrape together.
*
* A coin type from other Mario games that activates an event once all
* red coins (usually 8) are collected. Typically a star would spawn.
*
* Some parts of this actor are initialized through ActorInfo.
*
* Must be linked to a RedCoinController through Group ID.
*
* Credits:
* Evanbowl, Lord-Giganticus, Galaxy Master, and Aurum for helping me with crash fixes.
*/

/* --- RED COIN --- */
RedCoin::RedCoin(const char* pName) : Coin(pName) {
    mIsCollected = false;
    mLaunchVelocity = 250.0f;
    mUseConnection = false;
    mIsInAirBubble = false;
    mInvalidateShadows = false;
    mHasRewardedCoins = false;
    mAppearDelay = 0;
    mElapsed = 0;
    mRedCoinCounterPlayerPos = false;

    mIsPurple = false;
    mShadowCalcOn = true;
    mIgnoreGravity = false;
    mCalcShadowPrivateGravity = true;

    // Setup Coin
    MR::createCoinRotater();
    MR::createCoinHolder();
    MR::addToCoinHolder(this, this);
}

void RedCoin::init(const JMapInfoIter& rIter) {
    MR::processInitFunction(this, rIter, false);
    MR::joinToGroupArray(this, rIter, "RedCoin", 24);
    MR::calcGravity(this);

    MR::getJMapInfoArg0NoInit(rIter, &mLaunchVelocity); // Y Appear Launch Velocity. Calculates gravity.
    MR::getJMapInfoArg1NoInit(rIter, &mUseConnection); // Use MapObjConnection?
    MR::getJMapInfoArg2NoInit(rIter, &mIsInAirBubble); // Use AirBubble?
    MR::getJMapInfoArg3NoInit(rIter, &mInvalidateShadows); // Hide Shadows?
    MR::getJMapInfoArg4NoInit(rIter, &mAppearDelay); // SW_B Appear Spawn Delay
    MR::getJMapInfoArg5NoInit(rIter, &mRedCoinCounterPlayerPos);
    
    initNerve(&NrvCoin::CoinNrvFix::sInstance, 0);

    initHitSensor(1);
    MR::addHitSensor(this, "RedCoin", 0x4A, 4, 55.0f, TVec3f(0.0f, 70.0f, 0.0f));

    MR::initShadowVolumeSphere(this, 50.0f);
    MR::setShadowDropPositionPtr(this, 0, &mShadowDropPos);
    MR::setShadowDropLength(this, 0, 1000.0f);

    mFlashingCtrl = new FlashingCtrl(this, 1);

    mConnector = new MapObjConnector(this);
    mConnector->attach(mTranslation);

    makeActorAppeared();

    // Can't use ActorInfo for this one...
    MR::useStageSwitchSyncAppear(this, rIter);
}

void RedCoin::initAfterPlacement() {
    if (!mUseConnection)
        MR::offBind(this);

    if (MR::isValidSwitchB(this)) {
        MR::hideModel(this);
        MR::invalidateHitSensors(this);
    }

    if (mInvalidateShadows)
        MR::invalidateShadowAll(this);

    initAirBubble();
}


void RedCoin::control() {
    if (MR::isOnSwitchB(this) && MR::isHiddenModel(this) && !mIsCollected) {
        mElapsed++;
        
    if (mElapsed >= mAppearDelay)
        appearAndMove();
    }

    if (MR::isOnSwitchB(this))
        mLifeTime = 0x7FFFFFFF;

    if (mIsCollected)
        MR::zeroVelocity(this);
}

void RedCoin::calcAndSetBaseMtx() {
    if (mUseConnection && !mIsCollected) {
        mConnector->connect();
        mConnector->attachToUnder();
    }

    Coin::calcAndSetBaseMtx();
}

bool RedCoin::receiveMessage(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
	if (MR::isMsgItemGet(msg) && !mIsCollected) {
		collect();
        return true;
    }
    
	return false;
}

void RedCoin::initAirBubble() {
    if (mIsInAirBubble && !mUseConnection && !MR::isValidSwitchB(this)) {
        mAirBubble = MR::createPartsModelNoSilhouettedMapObj(this, "空気アワ", "AirBubble", 0);
        mAirBubble->initFixedPosition(TVec3f(0.0f, 70.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), 0);
        MR::startAction(mAirBubble, "Move");
        MR::setSensorRadius(this, "RedCoin", 100.0f);
    }
}

void RedCoin::appearAndMove() {
    // I need a better way to calculate the gravity
    TVec3f coinVelocity = TVec3f(0.0f, mLaunchVelocity / 10.0f, 0.0f);
    coinVelocity.scale(coinVelocity.y, -mGravity);
    
    appearMove(mTranslation, coinVelocity, 300, 60);
    MR::startSystemSE("SE_SY_RED_COIN_APPEAR", -1, -1);
}

void RedCoin::collect() {
    mIsCollected = true;

    RedCoinController* pController;
    LiveActorGroup* group = MR::getGroupFromArray(this);
    
    for (s32 i = 0; i < group->mNumObjs; i++) {
        if (MR::isEqualString(group->getActor(i)->mName, "RedCoinController")) {
            pController = (RedCoinController*)group->getActor(i);
            break;
        }
    }
    
    if (MR::isValidSwitchA(this))
        MR::onSwitchA(this);
    
    if (mIsInAirBubble) {
        MR::emitEffect(mAirBubble, "RecoveryBubbleBreak");
        mAirBubble->kill();
    }
    
    // Only ever increment coins once.
    if (!mHasRewardedCoins && !MR::isGalaxyDarkCometAppearInCurrentStage()) {
        MR::incPlayerLife(1);
        GameSequenceFunction::getPlayResultInStageHolder()->addCoinNum(!pController->mRewardCoins ? 2 : 0);
        mHasRewardedCoins = true;
    }

    pController->startCountUp(this);

    MR::startSystemSE(pController->mHasAllRedCoins ? "SE_SY_RED_COIN_COMPLETE" : "SE_SY_RED_COIN", -1, -1);

    MR::incPlayerOxygen(mIsInAirBubble ? 2 : 1);
    MR::invalidateHitSensors(this);
    MR::invalidateShadowAll(this);
    MR::emitEffect(this, "RedCoinGet");
    MR::hideModel(this);
}