/*
 *  DollBlob.h
 *  ImageTextureTest
 *
 *  Created by William Lindmeier on 4/5/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "cinder/Vector.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#define PI				3.141592653589793238462643383
#define TwoPI			(PI * 2.0)
#define HalfPI			(PI * 0.5)
#define OneAndHalfPI	(PI * 1.5)

ci::Vec2f rotateVecByRadians(const ci::Vec2f &vec, const float radians);

using namespace ci;

class DollBlob {
	
public: 
	
	DollBlob(std::string imagePath);
	void draw();
	void update(const ci::Vec3f &anchor, float scale);
	void setImage(ci::ImageSourceRef imageSource);
	
	float			mScale;
	ci::Vec2f		mVecAngle;
	ci::Vec3f		mVecAnchor;
	gl::Texture		mTexImage;
	float			mImageWidth, mImageHeight;
	ci::Vec3f		mGlVec0, mGlVec1, mGlVec2, mGlVec3;
	
};