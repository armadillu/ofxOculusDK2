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
    RUI_GET_INSTANCE()->setShowUIDuringEdits(true); //always show UI/HUD, even when editing
    RUI_NEW_GROUP("OVERLAY");
	RUI_SHARE_PARAM(showOverlay);
	RUI_SHARE_PARAM(showOverlay);
	RUI_SHARE_PARAM_WCN("unlockHudFromView", oculusRift.lockView);
    RUI_SHARE_PARAM(hudW, 200, 1920);
    RUI_SHARE_PARAM(hudH, 120, 1080);
    RUI_SHARE_PARAM(hudZ, -1300, 200);
    RUI_NEW_GROUP("JOYSTICK");
	RUI_SHARE_PARAM(debugJoystick);

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
	cam.setPosition(0,20,0);
    cam.lookAt(ofVec3f(20, 10.0, 0.0));

    for(int i = 0; i < 50; i++){
        DemoSphere d;
        d.color = ofColor(ofRandom(255),
                          ofRandom(255),
                          ofRandom(255));

        d.pos = ofVec3f(ofRandom(-500, 500),0,ofRandom(-500,500));

        d.floatPos.x = d.pos.x;
        d.floatPos.z = d.pos.z;

        d.radius = 20;

        d.bMouseOver = false;
        d.bGazeOver  = false;

        demos.push_back(d);
    }

    TIME_SAMPLE_GET_INSTANCE()->setAutoDraw(false);
    TIME_SAMPLE_GET_INSTANCE()->setDesiredFrameRate(75);

//	//enable mouse;
    cam.begin();
    cam.end();
}

void testApp::appplyJoystickToCam(ofCamera & camera){
	ofVec3f pos = camera.getPosition();
	int joystickID = 0;
	pos.z += ofxGLFWJoystick::one().getAxisValue(0, joystickID);
	pos.x += ofxGLFWJoystick::one().getAxisValue(1, joystickID);
	//these are -1 when resting
	float rightTrigger = ofMap(ofxGLFWJoystick::one().getAxisValue(5, joystickID), -1, 1, 0, 1);
	float leftTrigger = ofMap(ofxGLFWJoystick::one().getAxisValue(4, joystickID), -1, 1, 0, 1);
	//cout << leftTrigger << ", " << rightTrigger << endl;
	pos.y += rightTrigger - leftTrigger; //right increases y, left decreases y
	camera.setPosition(pos);
}


void testApp::update()
{

	ofxGLFWJoystick::one().update();



    for(int i = 0; i < demos.size(); i++){
        demos[i].floatPos.y = ofSignedNoise(ofGetElapsedTimef()/10.0,
											demos[i].pos.x/100.0,
											demos[i].pos.z/100.0,
											demos[i].radius*100.0) * demos[i].radius*20.;

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

}


//--------------------------------------------------------------
void testApp::draw()
{

	//float t = ofGetElapsedTimef();

    if(oculusRift.isSetup()){

        if(showOverlay){
			TS_START_ACC("overlay");
			ofSetColor(255);
			ofDisableLighting();
            oculusRift.beginOverlay(hudZ, hudW, hudH);
            ofRectangle overlayRect = oculusRift.getOverlayRectangle();
            TIME_SAMPLE_GET_INSTANCE()->setPlotBaseY(hudH);
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
	//cout << (ofGetElapsedTimef() - t) * 1000.0 << endl;
}

//--------------------------------------------------------------
void testApp::drawScene(){

	TS_START_ACC("drawScene");
    ofPushMatrix();
    ofRotate(90, 0, 0, -1);
    ofDrawGridPlane(300.0f, 30.0f, false );
    ofPopMatrix();
    light.enable();

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

    ofDrawAxis(50);

    //billboard and draw the mouse
    if(oculusRift.isSetup()){
        ofPushMatrix();
        oculusRift.multBillboardMatrix();
        ofSetColor(255, 0, 0);
        ofCircle(0,0,.5);
        ofPopMatrix();
    }

    ofPopStyle();
	TS_STOP_ACC("drawScene");
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{

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
	//   cursor2D.set(x, y, cursor2D.z);
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{
	//    cursor2D.set(x, y, cursor2D.z);
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