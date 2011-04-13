/***
	SkeletonApp

	A sample app showing skeleton rendering with the kinect and openni.
	This sample renders only the user with id=1. If user has another id he won't be displayed.
	You may change that in the code.

	V.
***/


#include "cinder/app/AppBasic.h"
#include "cinder/imageio.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Rand.h"
#include "VOpenNIHeaders.h"
#include "AvatarSkeleton.h"
#include <boost/lexical_cast.hpp>
#include "cinder/params/Params.h"
#include "cinder/ip/Resize.h"

#define USE_PARAMS 1

using namespace ci;
using namespace ci::app;
using namespace std;

class ImageSourceKinectColor : public ImageSource 
{
public:
	ImageSourceKinectColor( uint8_t *buffer, int width, int height )
		: ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_RGB );
		setChannelOrder( ImageIo::RGB );
		setDataType( ImageIo::UINT8 );
	}

	~ImageSourceKinectColor()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
			delete[] mData;
			mData = NULL;
		}*/
	}

	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );

		for( uint32_t row	 = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width * 3 );
	}

protected:
	uint32_t					_width, _height;
	uint8_t						*mData;	
};


class ImageSourceKinectDepth : public ImageSource 
{
public:
	ImageSourceKinectDepth( uint16_t *buffer, int width, int height )
		: ImageSource(), mData( buffer ), _width(width), _height(height)
	{
		setSize( _width, _height );
		setColorModel( ImageIo::CM_GRAY );
		setChannelOrder( ImageIo::Y );
		setDataType( ImageIo::UINT16 );
	}

	~ImageSourceKinectDepth()
	{
		// mData is actually a ref. It's released from the device. 
		/*if( mData ) {
			delete[] mData;
			mData = NULL;
		}*/
	}

	virtual void load( ImageTargetRef target )
	{
		ImageSource::RowFunc func = setupRowFunc( target );

		for( uint32_t row = 0; row < _height; ++row )
			((*this).*func)( target, row, mData + row * _width );
	}

protected:
	uint32_t					_width, _height;
	uint16_t					*mData;
};


class BlockOpenNISampleAppApp : public AppBasic 
{
public:
	static const int WIDTH = 640; //1280;
	static const int HEIGHT = 480; //720;

	// NOTE: Dont change these
	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 15;//30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
	static const int KINECT_DEPTH_FPS = 30;

	void prepareSettings( Settings* settings );
	void setup();
	void shutdown();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void keyDown( KeyEvent event );
//	bool addNewAvatar(float scale=1.0);
	bool iterateAvatar();
	
	ImageSourceRef getColorImage()
	{
		// register a reference to the active buffer
		uint8_t *activeColor = _device0->getColorMap();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}

	ImageSourceRef getUserColorImage( int id )
	{
		V::OpenNIUserRef user = _manager->getUser(id);

		// register a reference to the active buffer
		uint8_t *activeColor = user->getPixels();
		return ImageSourceRef( new ImageSourceKinectColor( activeColor, KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT ) );
	}

	ImageSourceRef getDepthImage()
	{
		// register a reference to the active buffer
		uint16_t *activeDepth = _device0->getDepthMap();
		return ImageSourceRef( new ImageSourceKinectDepth( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	} 

	ImageSourceRef getDepthImage24()
	{
		// register a reference to the active buffer
		uint8_t *activeDepth = _device0->getDepthMap24();
		return ImageSourceRef( new ImageSourceKinectColor( activeDepth, KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT ) );
	}

public:	// Members
	V::OpenNIDeviceManager*	_manager;
	V::OpenNIDevice::Ref	_device0;

	gl::Texture				mColorTex;
	gl::Texture				mDepthTex;
//	gl::Texture				mOneUserTex;	 
	
	AvatarSkeleton			*mAvatar;
	int						mCurrentAvatarIndex;
	//std::vector<AvatarSkeleton *> mAvatars;
	float					mScale, mHandsDistance;
	bool					mShouldRenderSkeleton, mIsTracking;
	long					mAppAge, mAgeOfLastClap;
	
	// Head masking
	float			mXCalibration, mYCalibration, mScaleCalibration;
	float			mXOffsetDepthMultiplier, mYOffsetDepthMultiplier;
	
#if USE_PARAMS == 1
	params::InterfaceGl	mParams;
#endif

};

void BlockOpenNISampleAppApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize( WIDTH, HEIGHT );
	mXCalibration = -22.0;
	mYCalibration = -34.0;
	mScaleCalibration = 1.08;		
	mXOffsetDepthMultiplier = 0.0;
	mYOffsetDepthMultiplier = 0.0;

#if USE_PARAMS == 1	 
	mParams = params::InterfaceGl("Calibration", Vec2i(200,100));

	mParams.addParam( "mXCalibration", &mXCalibration, "min=-1000.0 max=1000.0 step=1.0 keyIncr=2 keyDecr=1");
	mParams.addParam( "mYCalibration", &mYCalibration, "min=-1000.0 max=1000.0 step=1.0 keyIncr=0 keyDecr=9");
	mParams.addParam( "mScaleCalibration", &mScaleCalibration, "min=-10.0 max=10.0 step=0.01 keyIncr=s keyDecr=a");
	mParams.addParam( "mXOffsetDepthMultiplier", &mXOffsetDepthMultiplier, "min=-100.0 max=100.0 step=0.5 keyIncr=x keyDecr=z");
	mParams.addParam( "mYOffsetDepthMultiplier", &mYOffsetDepthMultiplier, "min=-100.0 max=100.0 step=0.5 keyIncr=y keyDecr=h");
#endif
	
	mCurrentAvatarIndex = -1;
	mAvatar	= NULL;
	mScale = 112.0;
	mShouldRenderSkeleton = false;
	mAppAge = 0;
	mAgeOfLastClap = 0;
	mHandsDistance = -1;
}
	
void BlockOpenNISampleAppApp::setup()
{	
	iterateAvatar();
	
	_manager = V::OpenNIDeviceManager::InstancePtr();
	_device0 = _manager->createDevice( "data/configIR.xml", true );
	if( !_device0 ) 
	{
		DEBUG_MESSAGE( "(App)  Couldn't init device0\n" );
		exit( 0 );
	}
	_device0->setPrimaryBuffer( V::NODE_TYPE_DEPTH );
	_manager->start();

	gl::Texture::Format format;
	gl::Texture::Format depthFormat;
	mColorTex = gl::Texture( KINECT_COLOR_WIDTH, KINECT_COLOR_HEIGHT, format );
	mDepthTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
	//mOneUserTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
}

#define mNumMaxAvatars	4

bool BlockOpenNISampleAppApp::iterateAvatar()
{
	mCurrentAvatarIndex = (mCurrentAvatarIndex + 1) % mNumMaxAvatars;
	if(mAvatar){
		delete mAvatar;
	}
	string avatarPrefix = "sk" + boost::lexical_cast<string>(mCurrentAvatarIndex) + "_";
	mAvatar = new AvatarSkeleton(avatarPrefix);	
	return true;
}

/*
bool BlockOpenNISampleAppApp::addNewAvatar(float scale)
{
	
	
	int avatarCount = mAvatars.size();
	if(avatarCount < mNumMaxAvatars){
		string avatarPrefix = "sk" + boost::lexical_cast<string>(avatarCount) + "_";
		// TODO: Use Z as a distance (scale) multiplier
		Vec3f positionOffset = Vec3f(200.0 * avatarCount, 0.0, scale);
		AvatarSkeleton *avatar = new AvatarSkeleton(avatarPrefix, positionOffset);
		mAvatars.push_back(avatar);
		
		// Sort the avatars by their depth
		// The depth is compared by the "<" operator
		sort (mAvatars.begin(), mAvatars.end());
		
		return true;
	}
	return false;
}
*/
void BlockOpenNISampleAppApp::shutdown()
{
	//delete mAvatar;
	/*
	for( std::vector<AvatarSkeleton*>::iterator it = mAvatars.begin(); it != mAvatars.end(); it++){
		AvatarSkeleton *avatar = *it;
		delete avatar;
	}
	mAvatars.clear();
	 */
	delete mAvatar;
}

void BlockOpenNISampleAppApp::mouseDown( MouseEvent event )
{
}

void BlockOpenNISampleAppApp::update()
{	
	++mAppAge;
	
	// Update textures
	ImageSourceRef colorImage = getColorImage();
	ImageSourceRef depthImage = getDepthImage24();
	mColorTex.update( colorImage );
	mDepthTex.update( depthImage );	// Histogram

	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ){ 

		//mOneUserTex.update( getUserColorImage(1) );
				
		xn::DepthGenerator* depth = _device0->getDepthGenerator();
		
		V::OpenNIBoneList boneList = _manager->getUser(1)->getBoneList();
		
		mIsTracking = _manager->getUser(1)->getUserState() == V::USER_TRACKING;
		//console() << "mIsTracking ? " << (mIsTracking ? "YES" : "NO") << "\n";
		
		if(mIsTracking){
			
			std::vector<Vec2f> joints;
			
			int index = 0;
			
			joints.push_back(Vec2f(0,0)); // This list starts at 1, so I have to pad joints
			
			for( std::vector<V::OpenNIBone*>::iterator it = boneList.begin(); it != boneList.end(); it++, index++ )
			{
				V::OpenNIBone* bone = *it;			
				// Convert a point from world coordinates to screen coordinates
				XnPoint3D point;
				XnPoint3D realJointPos;
				realJointPos.X = bone->position[0];
				realJointPos.Y = bone->position[1];
				realJointPos.Z = bone->position[2];
				depth->ConvertRealWorldToProjective( 1, &realJointPos, &point );			
				
				joints.push_back(Vec2f(point.X, point.Y));
			}

			mAvatar->update(joints, mScale);	
			/*
			for( std::vector<AvatarSkeleton*>::iterator it = mAvatars.begin(); it != mAvatars.end(); it++){
				AvatarSkeleton *avatar = *it;
				avatar->update(joints, mScale);	
			}		
			*/
			
#define	kMaxHandsDistanceForClap	25.0f
#define	kMinTimeBetweenClaps		120.0f
		// Check for "clap"

			Vec2f leftHandJoint = joints[V::SKEL_LEFT_HAND];
			Vec2f rightHandJoint = joints[V::SKEL_RIGHT_HAND];
			Vec2f headJoint = joints[V::SKEL_HEAD];
			Vec2f handsDelta = leftHandJoint - rightHandJoint;
			float newHandsDistance = handsDelta.length();
			// The big condition
			if(newHandsDistance < kMaxHandsDistanceForClap &&
			   mHandsDistance >= kMaxHandsDistanceForClap &&
//			   (mAppAge - mAgeOfLastClap) > kMinTimeBetweenClaps && 
			   leftHandJoint.y < headJoint.y && 
			   rightHandJoint.y < headJoint.y){
				// TODO: If BOB is on the stage, check if the hands are in proximity to the head joint
				// That would be a "hat" gesture
				
				// Dis is a clap sun
				if(iterateAvatar()){
					mAgeOfLastClap = mAppAge;
				}
			}
			mHandsDistance = newHandsDistance;		
			
			
			// Grab the users head

#define kFieldWidth			KINECT_COLOR_WIDTH
#define kFieldHeight		KINECT_COLOR_HEIGHT
			
			Surface rgbImage(colorImage);
			
			// Resize and crop the rgb to callibrate

			// TODO: Maybe this is better as high-res

			Surface resizedColorSurface(kFieldWidth, kFieldHeight, false);
			int x1 = mXCalibration / mScaleCalibration * -1.0;
			int y1 = mYCalibration / mScaleCalibration * -1.0;
			int x2 = x1 + (kFieldWidth / mScaleCalibration);
			int y2 = x1 + (kFieldHeight / mScaleCalibration);
			ci::ip::resize(rgbImage, Area(x1,y1,x2,y2), &resizedColorSurface, Area(0,0,kFieldWidth,kFieldHeight));
		
			Channel8u depthChannel(depthImage);// = mKinect.getDepthImage();
			//		Surface8u rgbImage = mKinect.getVideoImage();
			//		int width = rgbImage.getWidth();//
			//		int height = rgbImage.getHeight();
			
			// NOTE: We could make this arbitrarily bigger
			int halfHeadWidth = 80.0 * mAvatar->mScale; //(mAvatar->mHead->mImageWidth * mAvatar->mScale) / 2;
			int halfHeadHeight = 80.0 * mAvatar->mScale; //(mAvatar->mHead->mImageHeight * mAvatar->mScale) / 2;
			Vec2f headAnchor = joints[V::SKEL_HEAD];
			int startX = headAnchor.x - halfHeadWidth;
			int endX = headAnchor.x + halfHeadWidth;
			int startY = headAnchor.y - halfHeadHeight;
			int endY = headAnchor.y + halfHeadHeight;
		
			Surface8u headImage(halfHeadWidth * 2, halfHeadHeight * 2, true);

			// TMP: Just grabbing the depth head
			for(int y=startY;y<endY;++y){
				for(int x=startX;x<endX;++x){
					// Just grab the small head slice
					Vec2i cropPos(x-startX, y-startY);
					Vec2i dataPos(x, y);
					//float a = (float)depthChannel.getValue(pos) / 255.0;
					int depthValue = depthChannel.getValue(dataPos);
					Color8u rgbPix = resizedColorSurface.getPixel(dataPos);
					//int alpha = (depthValue > 150) ? 255 : 0;
					ColorA8u rgbaPix(rgbPix.r, rgbPix.g, rgbPix.b, depthValue);
					//ColorA8u rgbaPix(depthValue, depthValue, depthValue, depthValue);
					headImage.setPixel(cropPos, rgbaPix);
				}
			}
								
			// Reset the head image
			mAvatar->mHead->setImage(ImageSourceRef(headImage));
			
			/*
			for(int y=0;y<kFieldHeight;++y){
				for(int x=0;x<kFieldWidth;++x){
					Vec2i pos = Vec2i(x,y);
					float a = (float)depthChannel.getValue(pos) / 255.0;				
					Vec2i rgbPos(pos.x + (a * mXOffsetDepthMultiplier), pos.y + (a * mYOffsetDepthMultiplier));
					Color8u rgbPix = mRGBSurface.getPixel(rgbPos);
					Color8u rgbaPix(rgbPix.r * a, rgbPix.g * a, rgbPix.b * a);
					rgbDepth.setPixel(pos, rgbaPix);
				}
			}
			*/
					
		}
	}
}

void BlockOpenNISampleAppApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ), true ); 

	gl::setMatricesWindow( 640, 480 );

	glEnable( GL_TEXTURE_2D );
	gl::color( cinder::ColorA(1, 1, 1, 1) );

	// Draw the green user texture
	if( _manager->hasUsers() && _manager->hasUser(1) ){ 

		/*
		if(!mIsTracking){
			// Only draw the user texture if we're not tracking 
			gl::draw( mOneUserTex, Rectf( 0, 0, 640, 480) );		
		}
		 */
	}

	// Draw the info panels
	float sx = 320/2;
	float sy = 240/2;
	float xoff = 10;
	float yoff = 10;		
	gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
	gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );
	// TMP
	/*
	 This shows IR data
	uint8_t *data = _device0->mColorSurface->getData();
	ImageSourceRef colorImageSource(new ImageSourceKinectColor( data, 640, 480 ));
	gl::draw( gl::Texture(colorImageSource), Rectf( 0, 0, 640, 480) );
	 */
	
	if( _manager->hasUsers() && _manager->hasUser(1) )
	{
		if(mIsTracking){
			// Draw the avatar
			mAvatar->draw();
			/*
			for( std::vector<AvatarSkeleton*>::iterator it = mAvatars.begin(); it != mAvatars.end(); it++){
				AvatarSkeleton *avatar = *it;
				avatar->draw();
			}				
			*/
			// Draw the skeleton 
			if(mShouldRenderSkeleton){
				// Render skeleton if available
				_manager->renderJoints( 3 );
			}
		}
	}
	
#if USE_PARAMS == 1
	params::InterfaceGl::draw();
#endif
	
	// If the clap was recent, show a "flash"
	float ageOfLastClap = mAppAge - mAgeOfLastClap;
#define kFlashDuration	30.0f
	if((ageOfLastClap < kFlashDuration) && 
	   (mAppAge > kFlashDuration)){ // We dont want this to appear in the first frame
		float alpha = 1.0 - (ageOfLastClap / kFlashDuration);
		gl::enableAlphaBlending();
		gl::color( cinder::ColorA(1, 1, 1, alpha) );
		gl::drawSolidRect(Rectf(0.0, 0.0, WIDTH, HEIGHT));		
	}
	
}

void BlockOpenNISampleAppApp::keyDown( KeyEvent event )
{
	switch (event.getCode()) {
		case KeyEvent::KEY_ESCAPE:
			this->quit();
			this->shutdown();
			break;
		case KeyEvent::KEY_DOWN:
			mScale -= 1.0;
			console() << "mScale: " << mScale << "\n";
			// NOTE: This should be tilt
			break;
		case KeyEvent::KEY_UP:
			mScale += 1.0;
			console() << "mScale: " << mScale << "\n";
			// NOTE: This should be tilt
			break;
		case KeyEvent::KEY_s:
			mShouldRenderSkeleton = !mShouldRenderSkeleton;
			break;
		default:
			break;
	}
}

CINDER_APP_BASIC( BlockOpenNISampleAppApp, RendererGl )
