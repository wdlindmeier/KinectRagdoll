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
	void update(std::vector<Vec2f> joints, const float scale);
	
	std::string				mName;
	std::vector<DollBone *>	mBones;
	DollTorso				*mTorso;
	DollHead				*mHead;
	float					mScale;
	
};