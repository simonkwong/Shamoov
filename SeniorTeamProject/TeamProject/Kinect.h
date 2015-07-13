#pragma once
#define KINECT_AVAILABLE
#include <windows.h>
#include <ole2.h>

#ifdef KINECT_AVAILABLE

#include "NuiApi.h"
#pragma comment(lib, "Kinect10.lib")
#endif
#include "OgreVector2.h"
#include "Receivers.h"

namespace Ogre
{
	class Overlay;
	class OverlayElement;
}

const float CALIBRATION_TIME = 4.0f;

class Kinect : public SessionListener
{

public:
	Kinect(void);
	~Kinect(void);

	HRESULT initSensor();

	void update(float);
	virtual void StartSession();
	virtual void EndSession();
	void addSkelListener(KinectSkelMsgr *listener);

	bool callibrating() { return mCallibrating; }
    void cancelCallibration();
	void performCallibration();
	bool getAutoCallibrate() { return mAutoCallibrate; }
	void setAutoCallibrate(bool autoCal) { mAutoCallibrate = autoCal; }

	bool getEnableKinect() { return mEnableKinect; }
	void setEnableKinect(bool enable) { mEnableKinect = enable; }

	void callibrate(float delay = 4.0f, std::function<void(void)> callback = NULL);
	void shutdown();

	bool standingOrSeated;
	bool fStandSeat() { return standingOrSeated; }

	std::vector<Ogre::Vector3> initialPositions;
	std::vector<Ogre::Vector3> getSkeletonNodes();
	void getSkeletonAngles(Ogre::Degree &angle, Ogre::Degree &angle2);
	Ogre::Vector3 getNodePositions() { return mSkelPositions[NUI_SKELETON_POSITION_COUNT]; }
	Ogre::Degree leftRightAngle() { return mLeftRightAngle; }
	Ogre::Degree frontBackAngle() { return mFrontBackAngle; }
	float fLeftRight() { return mLeftRightAngle.valueDegrees(); }
	float fFrontBack() { return mFrontBackAngle.valueDegrees(); }

	int detectArm();
	int detectJump();
	Ogre::Real detectSway();
	Ogre::Real detectLean();
	Ogre::Real detectTurn();

	Ogre::Real leanForwardMax;
	Ogre::Real leanBackMax;
	Ogre::Real swayLeftMax;
	Ogre::Real swayRightMax;
	Ogre::Real leftRotationMax;
	Ogre::Real rightRotationMax;
	Ogre::Real leftLegLiftMax;
	Ogre::Real rightLegLiftMax;
	Ogre::Real leftRightRotation;

protected:

	bool mEnableKinect;
	bool mAutoCallibrate;

	void updateKinectSkeleton( );
	std::vector<KinectSkelMsgr *> mSkelListeners;

#ifdef KINECT_AVAILABLE
	// Current kinect
	INuiSensor *            m_pNuiSensor;
	BSTR                    m_instanceId;
	HANDLE        m_hNextSkeletonEvent;
	DWORD         m_SkeletonIds[NUI_SKELETON_COUNT];
	DWORD         m_TrackedSkeletonIds[NUI_SKELETON_MAX_TRACKED_COUNT];
	HANDLE        m_hThNuiProcess;
	HANDLE        m_hEvNuiProcessStop;
	Ogre::Vector3 mSkelPositions[NUI_SKELETON_POSITION_COUNT];

	static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);
	DWORD WINAPI            Nui_ProcessThread();
#endif

	bool   recenterNext;
	bool   updateDelay; 
	float  mTimeSinceLastLog;
	Ogre::Vector2 baseVectorDelta;

	Ogre::Degree mLeftRightAngle;
	Ogre::Degree mFrontBackAngle;
	Ogre::Degree mLeftRightTrue;

	float mTimeSinceLastUpdate;
	float mCalibrationClock;

	Ogre::Overlay *mToso1Overlay;
	Ogre::Overlay *mToso2Overlay;
	Ogre::Overlay *mCallibrationOverlay;
	bool mCallibrating;
	Ogre::OverlayElement *mCallibrationText;
    std::function<void()>  mCallibrationFinishedCallback;


};

