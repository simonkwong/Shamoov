#include "Kinect.h"
#include "OgreVector2.h"
#include "OgreMath.h"
#include "OgreOverlay.h"
#include "OgreOverlayManager.h"
#include "OgreOverlayContainer.h"



Kinect::Kinect(void)
{
	mToso1Overlay = Ogre::OverlayManager::getSingleton().getByName("Kinect/Torso1");
	mToso2Overlay = Ogre::OverlayManager::getSingleton().getByName("Kinect/Torso2");
	mCallibrationText = Ogre::OverlayManager::getSingleton().getOverlayElement("Kinect/Calibrate2");

	mCallibrating = false;

	mCallibrationOverlay = Ogre::OverlayManager::getSingleton().getByName("Kinect/CallibrationOverlay");

	mTimeSinceLastUpdate = 5.0; // Start out uncallibrated ...

	mToso1Overlay->show();
	mToso2Overlay->show();

	mToso1Overlay->setScroll(0.85f, 0.8f);
	mToso2Overlay->setScroll(0.65f, 0.8f);

	standingOrSeated = true; //Starts Standing

	mEnableKinect = false;
	mAutoCallibrate = false;
}

std::vector<Ogre::Vector3>
Kinect::getSkeletonNodes()
{
	std::vector<Ogre::Vector3> nodes;
	for(Ogre::Vector3 sn : mSkelPositions)
	{
		nodes.push_back(sn);
	}

	return nodes;
}

void
Kinect::addSkelListener(KinectSkelMsgr *listener)
{
  mSkelListeners.push_back(listener);
}


Kinect::~Kinect(void)
{
}

HRESULT
Kinect::initSensor()
{
#ifdef KINECT_AVAILABLE
	HRESULT  hr;

	if ( !m_pNuiSensor  || true)
	{

		HRESULT hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

		if ( FAILED(hr) )
		{
			return hr;
		}

		m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
	}

	//DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;
	DWORD nuiFlags =  NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_DEPTH;
	hr = m_pNuiSensor->NuiInitialize( nuiFlags );


	if ( FAILED( hr ) )
	{
		return hr;
	}

	if ( HasSkeletalEngine( m_pNuiSensor ) )
	{
		m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

		if (standingOrSeated) //Standing T Sitting F
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0);
		else if (!standingOrSeated)
			hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
		if( FAILED( hr ) )
		{
			return hr;
		}
	}

	m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

	initialPositions = getSkeletonNodes();

	return hr;
#else
	return 0;
#endif

}


void Kinect::StartSession()
{
	mSessionStarted = true;
	mTimeSinceLastLog = 0.0f;

}
void Kinect::EndSession()
{
	mSessionStarted = false;

}

void 
Kinect::callibrate(float delay, std::function<void(void)> callback)
{
    mCallibrationFinishedCallback = callback;

	if (delay == 0)
	{
		recenterNext = true;
	}
	else
	{
		std::string message = "Callibration in ";

		long long printDelay = (long long) (delay + 0.99f);

		message.append(std::to_string(printDelay));

		message.append(" seconds");

		mCallibrationText->setCaption(message);
		mCalibrationClock = delay;
		mCallibrating = true;
		mCallibrationOverlay->show();
	}
}

void
Kinect::cancelCallibration()
{
    mCallibrating = false;
	mCallibrationFinishedCallback();
    // mCallibrationFinishedCallback = NULL; // Note:  Why doesn't this work?
    mCallibrationOverlay->hide();
   
}
void
Kinect::update(float time)
{
	mTimeSinceLastUpdate += time;
	if (mCallibrating)
	{
		if (mTimeSinceLastUpdate > 1.0)
		{
			mCallibrationText->setCaption("Kinect Cannot See You. \nPlease Adjust Position.");
		}
		else
		{

			mCalibrationClock -= time;
			if (mCalibrationClock <= 0)
			{
                if (mCallibrationFinishedCallback != NULL)
                {
                    mCallibrationFinishedCallback();
					mEnableKinect = true;
                }
				recenterNext = true;
				mCallibrating = false;	
				mCallibrationOverlay->hide();
				leftLegLiftMax = Ogre::Math::Abs(initialPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y - leftLegLiftMax);
				rightLegLiftMax = Ogre::Math::Abs(initialPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y - rightLegLiftMax);

				OutputDebugString("\n--------------------------------------------------");
				OutputDebugString("\nMAX FORWARD LEAN ANGLE: ");
				OutputDebugString(std::to_string(leanForwardMax).c_str());
				OutputDebugString("\nMAX BACK LEAN ANGLE: ");
				OutputDebugString(std::to_string(leanBackMax).c_str());
				OutputDebugString("\nMAX LEFT SWAY ANGLE: ");
				OutputDebugString(std::to_string(swayLeftMax).c_str());
				OutputDebugString("\nMAX RIGHT SWAY ANGLE: ");
				OutputDebugString(std::to_string(swayRightMax).c_str());
				OutputDebugString("\nMAX LEFT ROTATION: ");
				OutputDebugString(std::to_string(leftRotationMax).c_str());
				OutputDebugString("\nMAX RIGHT ROTATION: ");
				OutputDebugString(std::to_string(rightRotationMax).c_str());
				OutputDebugString("\nMAX LEFT LEG LIFT: ");
				OutputDebugString(std::to_string(leftLegLiftMax).c_str());
				OutputDebugString("\nMAX RIGHT LEG LIFT: ");
				OutputDebugString(std::to_string(rightLegLiftMax).c_str());
				OutputDebugString("\n--------------------------------------------------");
				OutputDebugString("\n");

			}
			else
			{
				performCallibration();
			}

		}
	}
	else if(mSessionStarted)
	{
		mTimeSinceLastLog += time;
		if(mTimeSinceLastLog >= 1.0)
		{
			float lr, fb, lrt;
			for (std::vector<KinectSkelMsgr *>::iterator it = mSkelListeners.begin(); it != mSkelListeners.end(); it++)
			{
				lr = mLeftRightAngle.valueDegrees();
				fb = mFrontBackAngle.valueDegrees();
				lrt = mLeftRightTrue.valueDegrees();
				(*it)->ReceiveSkelData(new SkelData(lr, fb, lrt, mSkelPositions));
			} 
			mTimeSinceLastLog = 0.0;
		}
	}
}

void
Kinect::performCallibration()
{
	std::string message = "Callibration Started \n Callibrating in ";

	long long printDelay = (long long) (mCalibrationClock + 0.99f);

	message.append(std::to_string(printDelay));

	message.append(" seconds.\n");

	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();


	if (mCalibrationClock <= 45 && mCalibrationClock > 40)
	{
		message.append("Please Stand Up Straight And Stand Still. FOLLOW The Prompt.");

		/* Getting Start/Initial Positions */
		
		initialPositions = skeletonNodes;

	}
	if (mCalibrationClock <= 40 && mCalibrationClock > 35)
	{
		message.append("LEAN FORWARD As Far As Possible Without Moving Your Arms.");
		if (fFrontBack() < leanForwardMax)
			leanForwardMax = Ogre::Math::Abs(fFrontBack());
		if (leanForwardMax < -90)
			leanForwardMax = 90;
	}
	if (mCalibrationClock <= 35 && mCalibrationClock > 30)
	{
		message.append("LEAN BACK As Far As Possible Without Bending Your Knees.");
		if (fFrontBack() > leanBackMax)
			leanBackMax = fFrontBack();
		if (leanBackMax > 90)
			leanBackMax = 90;
	}
	if (mCalibrationClock <= 30 && mCalibrationClock > 25)
	{
		message.append("SWAY LEFT As Far As Possible.");
		if (fLeftRight() < swayLeftMax)
			swayLeftMax = Ogre::Math::Abs(fLeftRight());
		if (swayLeftMax < -90)
			swayLeftMax = 90;
	}
	if (mCalibrationClock <= 25 && mCalibrationClock > 20)
	{
		message.append("SWAY RIGHT As Far As Possible.");
		if (fLeftRight() > swayRightMax)
			swayRightMax = fLeftRight();
		if (swayRightMax > 90)
			swayRightMax = 90;
	}
	if (mCalibrationClock <= 20 && mCalibrationClock > 15)
	{
		message.append("ROTATE CLOCKWISE As Far As Possible Without Moving Your Feet.");
		if (leftRightRotation > leftRotationMax)
			leftRotationMax = leftRightRotation;
		if (leftRotationMax > 90)
			leftRotationMax = 90;
	}
	if (mCalibrationClock <= 15 && mCalibrationClock > 10)
	{
		message.append("ROTATE COUNTER-CLOCKWISE As Far As Possible Without Moving Your Feet.");
		if (leftRightRotation < rightRotationMax)
			rightRotationMax = Ogre::Math::Abs(leftRightRotation);
		if (rightRotationMax < -90)
			rightRotationMax = 90;
	}
	if (mCalibrationClock <= 10 && mCalibrationClock > 5)
	{
		message.append("LIFT Your LEFT LEG As High As Possible.");
		if (skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y > leftLegLiftMax)
			leftLegLiftMax = skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y;
	}
	if (mCalibrationClock <= 5 && mCalibrationClock > 0)
	{
		message.append("LIFT Your RIGHT LEG As High As Possible.");
		if (skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y > rightLegLiftMax)
			rightLegLiftMax = skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	}

	mCallibrationText->setCaption(message);
}

/*
 

1)      Weight shifting – all directions, in sitting and standing

2)      Reaching

3)      Stepping – i.e., marching in place, stepping forward/back, out to the side, backwards, or stepping up onto an object (like a step)

4)      Turning in place

5)      Turning the head

6)      Kicking

7)      Sit to/from stand

8)      Squatting/lunging

9)      Throwing

10)   balancing on one leg, going up

11)   down a step, turning (90, 180 and 360 degrees)

12)   head movements (helpful for our vestibular patients.  We often have them do head turns while fixating eyes on a target

13)   interesting to have an audio piece that could be done with eyes closed or blindfolded.  Different sounds that cue different movements to improve balance without vision.

*/


void
Kinect::updateKinectSkeleton()
{
#ifdef KINECT_AVAILABLE
	NUI_SKELETON_FRAME SkeletonFrame = {0};

	bool bFoundSkeleton = false;

	detectArm();
	detectJump();
	detectLean();
	detectSway();
	detectTurn();

	if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) )
	{
		for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
		{
			if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED ||
				(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY)) 
			{
				bFoundSkeleton = true;
			}
		}
		//if (NUI_SKELETON_TRACKED == )
	}

	// no skeletons!
	if( !bFoundSkeleton )
	{
		return;
	}
	mTimeSinceLastUpdate = 0;


	// smooth out the skeleton data
	HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
	if ( FAILED(hr) )
	{
		return;
	}

	int x = 0;

	bool bSkeletonIdsChanged = false;
	for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
	{
		if ( m_SkeletonIds[i] != SkeletonFrame.SkeletonData[i].dwTrackingID )
		{
			m_SkeletonIds[i] = SkeletonFrame.SkeletonData[i].dwTrackingID;
			bSkeletonIdsChanged = true;
		}

		// Show skeleton only if it is tracked, and the center-shoulder joint is at least inferred.
		if ( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
			SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
		{



			NUI_SKELETON_DATA * pSkel =  &SkeletonFrame.SkeletonData[i];

			// TODO:  Check for     pSkel->eSkeletonPositionTrackingState[ JOINT ];
			Vector4 spine = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_SPINE];
			Vector4 shoulderPos = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER];
			Vector4 headPos = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
			Vector4 leftShoulder =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT];
			Vector4 rightShoulder =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
			Vector4 leftElbow =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_LEFT];
			Vector4 rightElbow =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ELBOW_RIGHT];
			Vector4 leftWrist =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_LEFT];
			Vector4 rightWrist =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_WRIST_RIGHT];
			Vector4 leftHand =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT];
			Vector4 rightHand =  pSkel->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];
			Vector4 leftHip = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_HIP_LEFT];
			Vector4 rightHip = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_HIP_RIGHT];
			Vector4 leftKnee = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_LEFT];
			Vector4 rightKnee = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_RIGHT];
			Vector4 leftAnkle = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_LEFT];
			Vector4 rightAnkle = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_RIGHT];
			Vector4 leftFoot = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_FOOT_LEFT];
			Vector4 rightFoot = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT];

			for(int i = 0 ; i < NUI_SKELETON_POSITION_COUNT; i++)
			{
				mSkelPositions[i] = Ogre::Vector3(pSkel->SkeletonPositions[i].x,pSkel->SkeletonPositions[i].y,pSkel->SkeletonPositions[i].z);
			}

			/*
			int x = 3;
			Ogre::Vector2 leftVector(leftShoulder.x - rightShoulder.x, leftShoulder.z - rightShoulder.z);
			leftVector.normalise();
			Ogre::Vector2 FrontVector(leftVector.y, -leftVector.x);

			Ogre::Vector2 BaseVector((leftElbow.x + rightElbow.x + leftShoulder.x + rightShoulder.x) / 4,
				(leftElbow.z + rightElbow.z + leftShoulder.z + rightShoulder.z) / 4);
			if (recenterNext)
			{
				recenterNext = false;
				baseVectorDelta = Ogre::Vector2(headPos.x, headPos.z) - BaseVector;
			}

			// Note:  headPos is a 3D point, baseVector is a 2D point, hence z/y confusion
			float xDisplacement = (headPos.x - BaseVector.x - baseVectorDelta.x) * leftVector.x + (headPos.z - BaseVector.y-baseVectorDelta.y) * leftVector.y;

			float xDisplacement2 = (headPos.x - BaseVector.x) * leftVector.x + (headPos.z - BaseVector.y) * leftVector.y;

			Ogre::Radian leftRightAngle1 = Ogre::Math::ATan2(-xDisplacement, headPos.y - shoulderPos.y + 0.5f);
			Ogre::Radian leftRightAngle2 = Ogre::Math::ATan2(-xDisplacement2, headPos.y - shoulderPos.y + 0.5f);
			mLeftRightAngle = leftRightAngle1 * 4;
			mLeftRightTrue = leftRightAngle2 * 4;

			float ZDisplacement = (headPos.x - BaseVector.x-baseVectorDelta.x) * FrontVector.x + (headPos.z - BaseVector.y-baseVectorDelta.y) * FrontVector.y;

			Ogre::Radian frontBackAngle1 = Ogre::Math::ATan2(ZDisplacement, headPos.y - shoulderPos.y + 0.5f);
			mFrontBackAngle = frontBackAngle1 * 4;
			*/

		}
		else if ( true && SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_POSITION_ONLY )
		{
		}
	}
	//mToso1Overlay->setRotate(Ogre::Radian(-mLeftRightAngle));

	mToso1Overlay->setRotate(Ogre::Radian(-mLeftRightAngle));
	mToso1Overlay->setScroll(0.85f, 0.8f);

	mToso2Overlay->setRotate(Ogre::Radian(-mFrontBackAngle));
	mToso2Overlay->setScroll(0.65f, 0.8f);
#endif
}

#ifdef KINECT_AVAILABLE
DWORD WINAPI Kinect::Nui_ProcessThread(LPVOID pParam)
{
	Kinect *pthis = (Kinect *) pParam;
	return pthis->Nui_ProcessThread();
}
#endif 

void
Kinect::shutdown()
{
#ifdef KINECT_AVAILABLE
	    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->NuiShutdown( );
    }
    if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->Release();
        m_pNuiSensor = NULL;
    }
#endif

}

#ifdef KINECT_AVAILABLE
//-------------------------------------------------------------------
// Nui_ProcessThread
//
// Thread to handle Kinect processing
//-------------------------------------------------------------------
DWORD WINAPI Kinect::Nui_ProcessThread()
{
	const int numEvents = 2;
	HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextSkeletonEvent };
	int    nEventIdx;


	// Main thread loop
	bool continueProcessing = true;
	while ( continueProcessing )
	{
		// Wait for any of the events to be signalled
		nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 5000 );

		// Process signal events
		switch ( nEventIdx )
		{
		case WAIT_TIMEOUT:
//			continueProcessing = false;
			continue;

			// If the stop event, stop looping and exit
		case WAIT_OBJECT_0:
			continueProcessing = false;
			continue;

		case WAIT_OBJECT_0 + 1:
			updateKinectSkeleton( );
			break;
		}

	}

	return 0;
}
#endif


Ogre::Real
Kinect::detectSway()
{
	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();

	Ogre::Vector3 hipCenter = skeletonNodes[NUI_SKELETON_POSITION_HIP_CENTER];
	Ogre::Vector3 shoulderCenter = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	Ogre::Real opposite = shoulderCenter.x - hipCenter.x;
	Ogre::Real adjacent = shoulderCenter.y;

	Ogre::Real swayAngle = atan2f(opposite, adjacent) * (180 / Ogre::Math::PI);

	mLeftRightAngle = swayAngle;
	/*
	Ogre::Degree leftRightAngle = Ogre::Degree(0);
	Ogre::Degree frontBackAngle = Ogre::Degree(0);

	getSkeletonAngles(leftRightAngle, frontBackAngle);
	
	//SWAY RIGHT
	if ((leftRightAngle.valueDegrees()) < -25) 
		return leftRightAngle.valueDegrees();

	//SWAY LEFT
	if ((leftRightAngle.valueDegrees()) > 25) 
		return leftRightAngle.valueDegrees();
	*/
	if (swayAngle < -10.0 || swayAngle > 10.0)
		return swayAngle;

	//NONE OF THE ABOVE
	else
		return 0.0;
}

Ogre::Real
Kinect::detectLean()
{
	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();

	Ogre::Vector3 hipCenter = skeletonNodes[NUI_SKELETON_POSITION_HIP_CENTER];
	Ogre::Vector3 shoulderCenter = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER];

	Ogre::Real opposite = shoulderCenter.z - hipCenter.z;
	Ogre::Real adjacent = shoulderCenter.y;

	Ogre::Real leanAngle = atan2f(opposite, adjacent) * (180 / Ogre::Math::PI);
	/*
	OutputDebugString("LEANING ANGLE: ");
	OutputDebugString(std::to_string(leanAngle).c_str());
	OutputDebugString("\n");
	*/

	mFrontBackAngle = leanAngle;

	/*
	Ogre::Degree leftRightAngle = Ogre::Degree(0);
	Ogre::Degree frontBackAngle = Ogre::Degree(0);

	getSkeletonAngles(leftRightAngle, frontBackAngle);

	//LEAN FORWARD
	if (frontBackAngle.valueDegrees() > 25) 
		return frontBackAngle.valueDegrees();

	//LEAN BACK
	if (frontBackAngle.valueDegrees() < -25) 
		return frontBackAngle.valueDegrees();
	*/

	if (leanAngle < -10.0 || leanAngle > 10.0)
		return leanAngle;

	//NONE OF THE ABOVE
	else
		return 0.0;
}

int
Kinect::detectArm()
{

	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();
		//ARMS IN FRONT
	if (skeletonNodes[NUI_SKELETON_POSITION_HAND_LEFT].z < skeletonNodes[NUI_SKELETON_POSITION_SPINE].z &&
		skeletonNodes[NUI_SKELETON_POSITION_HAND_RIGHT].z < skeletonNodes[NUI_SKELETON_POSITION_SPINE].z)
		return 0;
	
	//ARMS IN BACK
	if (skeletonNodes[NUI_SKELETON_POSITION_HAND_LEFT].z > skeletonNodes[NUI_SKELETON_POSITION_SPINE].z &&
		skeletonNodes[NUI_SKELETON_POSITION_HAND_RIGHT].z > skeletonNodes[NUI_SKELETON_POSITION_SPINE].z)
		return 1;
	
	//NONE OF THE ABOVE
	else
		return -1;
}


Ogre::Real
Kinect::detectTurn()
{
	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();

	Ogre::Real opposite, adjacent, rotation;
	rotation = 0;

	enum direction { LEFT, RIGHT, NEITHER};

	direction way = NEITHER;

	if ((skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_LEFT].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z) < -0.1)
	{
		opposite = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_LEFT].z;
		adjacent = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].x - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
		leftRightRotation = atan2f(opposite, adjacent) * (180/Ogre::Math::PI);
		way = LEFT;

	}
	else if ((skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z) < -0.1)
	{
		opposite = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;
		adjacent = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
		leftRightRotation = atan2f(opposite, adjacent) * (180/Ogre::Math::PI);
		way = RIGHT;
	}
	else
		way = NEITHER;

	/*---------------------------------*/

	if (way == RIGHT)
	{
		
		opposite = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_RIGHT].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;
		adjacent = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
		rotation = atan2f(opposite, adjacent) * (180/Ogre::Math::PI);

		return -rotation;
		
	}

	if (way == LEFT)
	{
		opposite = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].z - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_LEFT].z;
		adjacent = skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_CENTER].x - skeletonNodes[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
		rotation = atan2f(opposite, adjacent) * (180/Ogre::Math::PI);

		return -rotation;
	}
	
	else
		return 0.0;
}


int
Kinect::detectJump()
{
	std::vector<Ogre::Vector3> skeletonNodes = getSkeletonNodes();

	//LEFT FOOT LIFTED
	if (skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y > skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y + 0.1)
		return 0;
	//RIGHT FOOT LIFTED
	else if (skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y > skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y + 0.1)
		return 1;
	//JUMP
	/*
	else if ((Ogre::Math::Abs(initialPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y - skeletonNodes[NUI_SKELETON_POSITION_FOOT_LEFT].y) > 0.4) &&
		 (Ogre::Math::Abs(initialPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y - skeletonNodes[NUI_SKELETON_POSITION_FOOT_RIGHT].y) > 0.4))
		return 2;
	*/
	else
		return -1;
}

void
Kinect::getSkeletonAngles(Ogre::Degree &angle, Ogre::Degree &angle2)
{
	angle = leftRightAngle();
	angle2 = frontBackAngle();
}

