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
#include "VOpenNIHeaders.h"
#include "AvatarSkeleton.h"

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
	static const int WIDTH = 1280;
	static const int HEIGHT = 720;

	static const int KINECT_COLOR_WIDTH = 640;	//1280;
	static const int KINECT_COLOR_HEIGHT = 480;	//1024;
	static const int KINECT_COLOR_FPS = 30;	//15;
	static const int KINECT_DEPTH_WIDTH = 640;
	static const int KINECT_DEPTH_HEIGHT = 480;
	static const int KINECT_DEPTH_FPS = 30;


	void setup();
	void shutdown();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void keyDown( KeyEvent event );

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
	gl::Texture				mOneUserTex;	 
	
	AvatarSkeleton			*mAvatar;
	float					mScale;
	bool					mShouldRenderSkeleton, mIsTracking;
};

void BlockOpenNISampleAppApp::setup()
{
	mAvatar = new AvatarSkeleton("");
	mScale = 112.0;
	mShouldRenderSkeleton = false;
	
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
	mOneUserTex = gl::Texture( KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, format );
}

void BlockOpenNISampleAppApp::shutdown()
{
	delete mAvatar;
}

void BlockOpenNISampleAppApp::mouseDown( MouseEvent event )
{
}

void BlockOpenNISampleAppApp::update()
{	
	// Update textures
	mColorTex.update( getColorImage() );
	mDepthTex.update( getDepthImage24() );	// Histogram

	// Uses manager to handle users.
	if( _manager->hasUsers() && _manager->hasUser(1) ){ 
		mOneUserTex.update( getUserColorImage(1) );
				
		xn::DepthGenerator* depth = _device0->getDepthGenerator();
		
		V::OpenNIBoneList boneList = _manager->getUser(1)->getBoneList();
		
		mIsTracking = _manager->getUser(1)->getUserState() == V::USER_TRACKING;
		//console() << "mIsTracking ? " << (mIsTracking ? "YES" : "NO") << "\n";
		
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

		if(!mIsTracking){
			// Only draw the user texture if we're not tracking 
			gl::draw( mOneUserTex, Rectf( 0, 0, 640, 480) );		
		}
	}

	// Draw the info panels
	float sx = 320/2;
	float sy = 240/2;
	float xoff = 10;
	float yoff = 10;		
	gl::draw( mDepthTex, Rectf( xoff, yoff, xoff+sx, yoff+sy) );
	gl::draw( mColorTex, Rectf( xoff+sx*1, yoff, xoff+sx*2, yoff+sy) );

	if( _manager->hasUsers() && _manager->hasUser(1) )
	{
		// Draw the avatar
		mAvatar->draw();

		// Draw the skeleton 
		if(mShouldRenderSkeleton){
			// Render skeleton if available
			_manager->renderJoints( 3 );
		}
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
