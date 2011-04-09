/*
 *  DollTorso.h
 *  ImageTextureTest
 *
 *  Created by William Lindmeier on 4/5/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#import "DollBlob.h"
#include "cinder/Vector.h"

class DollTorso : public DollBlob{
	
public:
	
	DollTorso(std::string imagePath);
	void update(const ci::Vec3f &anchor, const ci::Vec2f &angle, const float scale);
	void draw();
	
	ci::Vec3f		mVecLeftShoulderAnchor, mVecRightShoulderAnchor, mVecLeftHipAnchor, mVecRightHipAnchor;
	
};