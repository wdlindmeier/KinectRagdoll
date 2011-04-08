/*
 *  AvatarSkeleton.h
 *  ImageTextureTest
 *
 *  Created by William Lindmeier on 4/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#import "DollBone.h"
#import "DollTorso.h"
#import "DollHead.h"

// TMP!
// Putting this here for the time being so we can fake data
/*
enum SkeletonBoneType
{
	SKEL_HEAD = 1, 
	SKEL_NECK = 2, 
	SKEL_TORSO = 3, 
	SKEL_WAIST = 4, 
	SKEL_LEFT_COLLAR = 5, 
	SKEL_LEFT_SHOULDER = 6, 
	SKEL_LEFT_ELBOW = 7, 
	SKEL_LEFT_WRIST = 8, 
	SKEL_LEFT_HAND = 9, 
	SKEL_LEFT_FINGERTIP = 10, 
	SKEL_RIGHT_COLLAR = 11, 
	SKEL_RIGHT_SHOULDER = 12, 
	SKEL_RIGHT_ELBOW = 13, 
	SKEL_RIGHT_WRIST = 14, 
	SKEL_RIGHT_HAND = 15, 
	SKEL_RIGHT_FINGERTIP = 16, 
	SKEL_LEFT_HIP = 17, 
	SKEL_LEFT_KNEE = 18, 
	SKEL_LEFT_ANKLE = 19, 
	SKEL_LEFT_FOOT = 20, 
	SKEL_RIGHT_HIP = 21, 
	SKEL_RIGHT_KNEE = 22, 
	SKEL_RIGHT_ANKLE = 23, 
	SKEL_RIGHT_FOOT = 24 
};
*/

enum AvatarBoneType
{
/*	TORSO_2_LEFT_SHOULDER = 0,
	TORSO_2_RIGHT_SHOULDER,
	TORSO_2_LEFT_HIP,
	TORSO_2_RIGHT_HIP,
	TORSO_2_WAIST,
	NECK_2_LEFT_SHOULTER,
	NECK_2_RIGHT_SHOULDER,
	LEFT_HIP_2_RIGHT_HIP,
	NECK_2_HEAD,*/
	LEFT_SHOULDER_2_LEFT_ELBOW = 0,
	LEFT_ELBOW_2_LEFT_HAND,
	RIGHT_SHOULDER_2_RIGHT_ELBOW,
	RIGHT_ELBOW_2_RIGHT_HAND,
	LEFT_HIP_2_LEFT_KNEE,
	LEFT_KNEE_2_LEFT_FOOT,
	RIGHT_HIP_2_RIGHT_KNEE,
	RIGHT_KNEE_2_RIGHT_FOOT //,
//	WAIST_2_NECK,
};

class AvatarSkeleton {

public:
	
	AvatarSkeleton(std::string resourceName);
	~AvatarSkeleton();
	void draw();
	void update(std::vector<Vec2f> joints);
	
	std::string				mName;
	std::vector<DollBone *>	mBones;
	DollTorso				*mTorso;
	DollHead				*mHead;
	
};