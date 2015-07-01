#pragma once

#include "ofMain.h"
#include "ofxOculusDK2.h"
#include "ofxGLFWJoystick.h"
#include "ofxAssimpModelLoader.h"
#include "ofxKinect.h"

#define MAX_FRAMES			2

typedef struct{
    ofColor color;
    ofVec3f pos;
    ofVec3f floatPos;
    float radius;
    bool bMouseOver;
    bool bGazeOver;
} DemoSphere;

class testApp : public ofBaseApp
{
public:

    void setup();
    void update();
    void draw();

    void drawScene();
	void drawKinectFrame(int);
	void appplyJoystickToCam(ofCamera & cam);

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ofxOculusDK2        oculusRift;
	ofxKinect			kinect;

    ofLight             light;
    ofEasyCam           cam;

	bool				showOverlay;
    bool				predictive;

    vector<DemoSphere> demos;
	ofxAssimpModelLoader * model;
	float modelScale;
	int numModels;

	//RUI params

	bool debugJoystick;
	float joystickGain;
    float hudZ;
    int hudW, hudH;

	int kinectDistThresholdMin, kinectDistThresholdMax;
	int kinectTiltAngle;
	bool kinectColors;
	float kinectScale;
	float kinectZOffset;
	float kinectYOffset;
	float kinectXOffset;
	bool kinectFlipZ;
	bool drawKinect;

	float noiseAmp;
	bool calcNoise;
	int sampleStepX, sampleStepY;
	float pointSize;
	float colorAlpha;

	int blurIterations;
	float blurOffset;
	int numBlurOverlays;
	int blurOverlayGain;


	vector< ofVboMesh* > frames;
};