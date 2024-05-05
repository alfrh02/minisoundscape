#pragma once

#include <vector>
#include "ofMain.h"
#include "ofxXmlSettings.h"

#define WIDTH  6
#define HEIGHT 4
#define GAP    0.1
#define ICON_Y 1.0

class ofApp : public ofBaseApp{

	public:
		~ofApp();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);

		ofEasyCam cam;

		ofShader spriteShader;

		ofMesh mTile;
		ofMesh mSpeaker;
		ofMesh mMicrophone;

		ofTexture texSpeaker;
		ofTexture texMicrophone;

		glm::vec3 posMicrophone = glm::vec3(0);

		bool ticking = true;
};