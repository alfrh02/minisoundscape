#pragma once

#include <vector>
#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{

	public:
		~ofApp();
		void setup();
		void update();
		void draw();
		void keyPressed(int key);

		short indexCurrentSoundscape = 0;
		bool ticking = true;
};