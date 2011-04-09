/*
 *  DollBone.h
 *  ImageTextureTest
 *
 *  Created by William Lindmeier on 4/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#import "DollBlob.h"

class DollBone : public DollBlob {
	
public:
	
	DollBone(std::string imagePath);
	void update(const ci::Vec3f &anchor, const ci::Vec2f &angle, const float scale);
	void draw();
	
	ci::Vec3f		mVecHook;
	
};