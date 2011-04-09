/*
 *  DollHead.h
 *  ImageTextureTest
 *
 *  Created by William Lindmeier on 4/5/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#import "DollBlob.h"

class DollHead : public DollBlob{
	
public:

	DollHead(std::string imagePath);
	void update(const ci::Vec3f &anchor, const Vec2f &angle, const float scale);
	void draw();
	
};