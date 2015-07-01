#include "testApp.h"
#include "ofxTimeMeasurements.h"
#include "ofxRemoteUIServer.h"

//--------------------------------------------------------------
void testApp::setup()
{
    ofBackground(0);
    //ofSetLogLevel( OF_LOG_VERBOSE );
    ofSetVerticalSync( true );
    ofSetFrameRate(77);
    ofSetFullscreen(false);

    RUI_SETUP();
    hudW = 320;
    hudH = 240;
    hudZ = -230;
    RUI_GET_INSTANCE()->setAutoDraw(false);
    //RUI_GET_INSTANCE()->setShowUIDuringEdits(true); //always show UI/HUD, even when editing
    RUI_NEW_GROUP("OVERLAY");
	RUI_SHARE_PARAM(showOverlay);
	RUI_SHARE_PARAM_WCN("unlockHudFromView", oculusRift.lockView);
    RUI_SHARE_PARAM(hudW, 200, 1920);
    RUI_SHARE_PARAM(hudH, 120, 1080);
    RUI_SHARE_PARAM(hudZ, -1300, 200);
    RUI_NEW_GROUP("JOYSTICK");
	RUI_SHARE_PARAM(debugJoystick);
	RUI_SHARE_PARAM(joystickGain, 0, 0.2);

	RUI_NEW_GROUP("MODEL");
	RUI_SHARE_PARAM(modelScale, 0.001, 1.0);
	RUI_SHARE_PARAM(numModels, 0, 10);


	RUI_NEW_GROUP("KINECT DATA SAMPLING");
	RUI_SHARE_PARAM(sampleStepX, 1, 30);
	RUI_SHARE_PARAM(sampleStepY, 1, 30);
	RUI_NEW_COLOR();
	RUI_SHARE_PARAM(calcNoise);
	RUI_SHARE_PARAM(noiseAmp, 0, 10);

	RUI_NEW_GROUP("VISUAL TWEAKS");
	RUI_SHARE_PARAM(pointSize, 1, 20);
//	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
//	OFX_REMOTEUI_SERVER_SHARE_PARAM(blurIterations, 0, 10);
//	OFX_REMOTEUI_SERVER_SHARE_PARAM(blurOffset, 0, 5);
//	OFX_REMOTEUI_SERVER_SHARE_PARAM(numBlurOverlays, 0, 10);
//	OFX_REMOTEUI_SERVER_SHARE_PARAM(blurOverlayGain, 0, 255);
//	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	RUI_SHARE_PARAM(colorAlpha, 0, 1);


	RUI_NEW_GROUP("KINECT SETTINGS");
	RUI_SHARE_PARAM(drawKinect);
	RUI_SHARE_PARAM(kinectDistThresholdMin, 0, 4500);
	RUI_SHARE_PARAM(kinectDistThresholdMax, 0, 7500);
	RUI_SHARE_PARAM(kinectTiltAngle, -30, 30);
	RUI_NEW_COLOR();
	RUI_SHARE_PARAM(kinectScale, 0, 1);
	RUI_SHARE_PARAM(kinectColors);
	RUI_NEW_COLOR();
	RUI_SHARE_PARAM(kinectXOffset, -10, 10);
	RUI_SHARE_PARAM(kinectYOffset, -10, 10);
	RUI_SHARE_PARAM(kinectZOffset, -10, 10);
	RUI_NEW_COLOR();
	RUI_SHARE_PARAM(kinectFlipZ);


    RUI_LOAD_FROM_XML();

	//    ofSetWindowPosition(1920, 0);
	//    ofToggleFullscreen();
    showOverlay = true;
    predictive = true;
    ofSetSphereResolution(10);

    ofHideCursor();

    //glEnable(GL_FOG);


    oculusRift.baseCamera = &cam;
    oculusRift.setup();
	oculusRift.dismissSafetyWarning();

	cam.setAutoDistance(false);
	//https://github.com/obviousjim/ofxOculusDK2/commit/1a71a4322ea048cef07d4bb65db6191abcb751a6
	cam.setPosition(1,0.5,0);
    cam.getTarget().setPosition(ofVec3f(0, 0, 0));

    for(int i = 0; i < 10; i++){
        DemoSphere d;
        d.color = ofColor(ofRandom(255),
                          ofRandom(255),
                          ofRandom(255));

		float rad = 2;
        d.pos = ofVec3f(ofRandom(-rad, rad),
						ofRandom(0, rad),
						ofRandom(-rad,rad));

        d.floatPos.x = d.pos.x;
        d.floatPos.z = d.pos.z;

        d.radius = 0.02;

        d.bMouseOver = false;
        d.bGazeOver  = false;

        demos.push_back(d);
    }

	model = new ofxAssimpModelLoader();
	model->loadModel("mario.3DS", true);
	model->setPosition(0, 0, 0);
	model->setRotation(0, 180, 1, 0, 0);
	model->setRotation(0, 180, 0, 0, 1);
	model->stopAllAnimations();

    TIME_SAMPLE_GET_INSTANCE()->setAutoDraw(false);
    TIME_SAMPLE_GET_INSTANCE()->setDesiredFrameRate(75);
	TIME_SAMPLE_SET_ENABLED(false);


	// enable depth->video image calibration
	kinect.setRegistration(true);

	kinect.init();
	//kinect.init(true); // shows infrared instead of RGB video image
	//kinect.init(false, false); // disable video image (faster fps)

	kinect.open();		// opens first available kinect
	//kinect.open(1);	// open a kinect by id, starting with 0 (sorted by serial # lexicographically))
	//kinect.open("A00362A08602047A");	// open a kinect using it's unique serial #

	// print the intrinsic IR sensor values
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}


//	enable mouse;
	cam.begin();
	cam.end();
}

void testApp::appplyJoystickToCam(ofCamera & camera){

	float th = 0.2;
	int joystickID = 0;
	float z = ofxGLFWJoystick::one().getAxisValue(0, joystickID);
	float x = ofxGLFWJoystick::one().getAxisValue(1, joystickID);
	if(fabs(x) > th){
		camera.dolly(joystickGain * x);
	}
	if(fabs(z) > th){
		camera.truck(joystickGain * z);
	}
	//these are -1 when resting
	float rightTrigger = ofMap(ofxGLFWJoystick::one().getAxisValue(5, joystickID), -1, 1, 0, 1);
	float leftTrigger = ofMap(ofxGLFWJoystick::one().getAxisValue(4, joystickID), -1, 1, 0, 1);
	float y = (rightTrigger - leftTrigger); //right increases y, left decreases y
	if(fabs(y) > th){
		camera.boom(joystickGain * y);
	}

	ofVec3f currLookTarget = camera.getPosition() + camera.getLookAtDir();
	camera.getLookAtDir();

	float dx = ofxGLFWJoystick::one().getAxisValue(2, joystickID);;
	float dy = ofxGLFWJoystick::one().getAxisValue(3, joystickID);;


	if(fabs(dx) > th || fabs(dy) > th){
		float lookG = 3;
		ofVec3f currentUp = camera.getUpDir();
		currLookTarget.rotate(-dx * lookG, camera.getPosition(), currentUp);
		ofVec3f sideVec = (currentUp).getCrossed(currLookTarget - camera.getPosition());
		camera.lookAt(currLookTarget, currentUp);
		currLookTarget.rotate(dy * lookG, camera.getPosition(), sideVec);
		currentUp = ( currLookTarget - camera.getPosition() ).getCrossed(sideVec);
		camera.lookAt(currLookTarget, currentUp);
	}

}

int c = 0;


void testApp::update(){

	kinect.update();
	kinect.setCameraTiltAngle(kinectTiltAngle);

	ofxGLFWJoystick::one().update();
	appplyJoystickToCam(cam);

    for(int i = 0; i < demos.size(); i++){
        demos[i].floatPos.y = ofNoise(ofGetElapsedTimef()/10.0,
											demos[i].pos.x,
											demos[i].pos.z,
											demos[i].radius);

    }

    if(oculusRift.isSetup()){
        ofRectangle viewport = oculusRift.getOculusViewport();
        for(int i = 0; i < demos.size(); i++){
            // mouse selection
            float mouseDist = oculusRift.distanceFromMouse(demos[i].floatPos);
            demos[i].bMouseOver = (mouseDist < 50);

            // gaze selection
            ofVec3f screenPos = oculusRift.worldToScreen(demos[i].floatPos, true);
            float gazeDist = ofDist(screenPos.x, screenPos.y,
                                    viewport.getCenter().x, viewport.getCenter().y);
            demos[i].bGazeOver = (gazeDist < 25);
        }
    }

	TS_START("kinect create mesh");
	if(kinect.isFrameNew() && drawKinect) {
		c++;
		int w = kinect.width;
		int h = kinect.height;
		ofVboMesh * mesh = new ofVboMesh();
		mesh->setMode(OF_PRIMITIVE_POINTS);
		int stepX = sampleStepX;
		int stepY = sampleStepY;

		for(int y = h-1; y >= 0; y -= stepY) {
			for(int x = 0; x < w - stepX; x += stepX) {

				if(kinect.getDistanceAt(x, y) > 0) {

					float off = noiseAmp;
					ofVec3f v = kinect.getWorldCoordinateAt(x, y);
					if (calcNoise){
						v += ofVec3f(ofRandom(-off, off),ofRandom(-off, off),ofRandom(-off, off));
					}

					if( v.z > kinectDistThresholdMin &&  v.z < kinectDistThresholdMax   ){
						if(kinectColors){
						mesh->addColor( ofColor(kinect.getColorAt(x,y),colorAlpha * 255 ) );
						}
						mesh->addVertex(v);
					}
				}
			}
		}

		frames.push_back(mesh);

		if (frames.size() > MAX_FRAMES){
			delete frames[0];
			frames.erase(frames.begin());
		}
	}
	TS_STOP("kinect create mesh");

}


//--------------------------------------------------------------
void testApp::draw(){

    if(oculusRift.isSetup()){

        if(showOverlay){
			TS_START_ACC("overlay");
			ofSetColor(255);
			ofDisableLighting();
            oculusRift.beginOverlay(hudZ, hudW, hudH);
            ofRectangle overlayRect = oculusRift.getOverlayRectangle();
            TIME_SAMPLE_GET_INSTANCE()->setPlotBaseY(hudH);
			RUI_GET_INSTANCE()->setCustomScreenHeight(hudH);
			TS_STOP_ACC("overlay");

            TIME_SAMPLE_GET_INSTANCE()->draw(0,0);

			if(debugJoystick){
				ofxGLFWJoystick::one().drawDebug(0,200);
			}

			TS_START_ACC("overlay");
            RUI_GET_INSTANCE()->draw(20, hudH - 20);
            oculusRift.endOverlay();
			ofEnableLighting();
			TS_STOP_ACC("overlay");
        }

        ofSetColor(255);
        glEnable(GL_DEPTH_TEST);

        TS_START("eyes Draw");
        oculusRift.beginLeftEye();
        drawScene();
        oculusRift.endLeftEye();

        oculusRift.beginRightEye();
        drawScene();
        oculusRift.endRightEye();

        glDisable(GL_DEPTH_TEST);
        TS_STOP("eyes Draw");

		TS_START("draw Rift");
        oculusRift.draw();
		TS_STOP("draw Rift");

    }
    else{
        cam.begin();
        drawScene();
        cam.end();
    }
}

//--------------------------------------------------------------
void testApp::drawScene(){

    ofPushMatrix();
	ofRotate(90, 0, 0, -1);
	ofSetColor(255,128);
	ofSetLineWidth(2);
    ofDrawGridPlane(4.0f, 8.0f, false );
    ofPopMatrix();
    light.enable();
	light.setPosition(0, 100, 0);
	//light.draw();
	ofSetColor(255);

    ofPushStyle();
    for(int i = 0; i < demos.size(); i++){
        ofPushMatrix();
        ofTranslate(demos[i].floatPos);

        if (demos[i].bMouseOver)
            ofSetColor(ofColor::white.getLerped(ofColor::red, sin(ofGetElapsedTimef()*10.0)*.5+.5));
        else if (demos[i].bGazeOver)
            ofSetColor(ofColor::white.getLerped(ofColor::green, sin(ofGetElapsedTimef()*10.0)*.5+.5));
        else
            ofSetColor(demos[i].color);

        ofSphere(demos[i].radius);
        ofPopMatrix();
    }

	model->setScaleNomalization(false);
	model->setScale(modelScale, modelScale, modelScale);
	int n = numModels;
	ofSetColor(255);
	for(int i = 0; i < n; i++){
		ofPushMatrix();
		ofTranslate( - (n - 1) * 0.5 + i , 0, -1.0);
		model->drawFaces();
		ofPopMatrix();
	}

    ofDrawAxis(0.2);

	//kinect
	if(drawKinect){
		glPointSize( pointSize );
		drawKinectFrame(0);
	}

    //billboard and draw the mouse
    if(oculusRift.isSetup()){
        ofPushMatrix();
        oculusRift.multBillboardMatrix();
        ofSetColor(255, 0, 0);
        ofCircle(0,0,.1);
        ofPopMatrix();
    }

    ofPopStyle();
}



void testApp::drawKinectFrame(int frame) {

	ofPushMatrix();
	// the projected points are 'upside down' and 'backwards'
	ofTranslate(kinectXOffset, kinectYOffset, -kinectZOffset); // center the points a bit
	float s = 0.01 * kinectScale;
	float zFlip = kinectFlipZ ? 1 : -1;
	ofScale(s, -s, -zFlip * s);
	//	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_FLAT);
	ofDisableLighting();
	if ( frame >= 0 && frame < frames.size() ){
		frames[frame]->drawVertices();
	}
	ofEnableLighting();
	glShadeModel(GL_SMOOTH);
	ofPopMatrix();

}


//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	oculusRift.dismissSafetyWarning();

    if( key == 'f' )
    {
        //gotta toggle full screen for it to be right
        ofToggleFullscreen();
    }

    if(key == 's'){
        oculusRift.reloadShader();
    }

    if(key == 'l'){
        oculusRift.lockView = !oculusRift.lockView;
    }

    if(key == 'o'){
        showOverlay = !showOverlay;
    }
    if(key == 'r'){
        oculusRift.reset();

    }
    if(key == 'h'){
        ofHideCursor();
    }
    if(key == 'H'){
        ofShowCursor();
    }

    if(key == 'p'){
        predictive = !predictive;
        oculusRift.setUsePredictedOrientation(predictive);
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}