#include "ofApp.h"

#define PATH(x) (ofToDataPath(x, true))

#define MS_NO_SPATIALIZATION
#define MS_DEFAULT_TICK_RATE 0.3
#include "minisoundscape.h"

/* All sounds in this example are made using JSFXR https://sfxr.me/ :) */

static ma_engine engine;
vector<ms_soundscape*> soundscapes;

//--------------------------------------------------------------
ofApp::~ofApp() {
	for (size_t i = 0; i < soundscapes.size(); i++) {
		ms_soundscape_uninit(soundscapes[i]);
	}
}

//--------------------------------------------------------------
void ofApp::setup(){
	ma_result result = ma_engine_init(NULL, &engine);
	if (result != MA_SUCCESS) printf("Failed to initialise engine.\n");

 	ofxXmlSettings settings;
	settings.load("settings.xml");
	for (int i = 0; i < settings.getNumTags("soundscape"); i++) {
		settings.pushTag("soundscape", i);

		ms_soundscape* soundscape = new ms_soundscape;
		ms_soundscape_init(
			settings.getValue("name", ""),
			&engine,
			PATH(settings.getValue("ambient", "")),
			soundscape
		);
		soundscapes.push_back(soundscape);

		settings.pushTag("sounds");
		for (int j = 0; j < settings.getNumTags("sound"); j++) {
			settings.pushTag("sound", j);

			ms_sound* sound = new ms_sound;
			if (settings.getValue("name", "") == "empty") {
				ms_sound_init_empty(
					sound,
					settings.getValue("weight", 0)
				);
			} else {
				ms_sound_init(
					settings.getValue("name", ""),
					&engine,
					settings.getValue("weight", 0),
					PATH(settings.getValue("filepath", "")),
					sound
				);
			}
			ms_sound_set_pan(
				sound,
				settings.getValue("panStart", 0),
				settings.getValue("panEnd", 0)
			);

			ms_soundscape_add_sound(soundscape, sound);
			settings.popTag();
		}
		settings.popTag(); // soundscape tag
		settings.popTag(); // sounds tag
	}

	// ms_soundscape_debug_list(soundscapes[indexCurrentSoundscape]);
	ms_soundscape_start(soundscapes[indexCurrentSoundscape]);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (ticking) ms_soundscape_tick(soundscapes[indexCurrentSoundscape]);
}

//--------------------------------------------------------------
void ofApp::draw(){
	stringstream str;
	str << "Press Q or E to cycle through available soundscapes" << endl;
	str << "Press space to toggle ticking" << endl;
	str << "Press x to play a sound" << endl;
	str << "Current soundscape: " << soundscapes[indexCurrentSoundscape]->name << endl;
	str << "Ticking: " << to_string(ticking) << endl;
	ofDrawBitmapString(str.str(), 8, 16);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (tolower(key)) {
		case 'q':
			indexCurrentSoundscape++;
			if (indexCurrentSoundscape > soundscapes.size() - 1) indexCurrentSoundscape = 0;
			break;
		case 'e':
			indexCurrentSoundscape--;
			if (indexCurrentSoundscape < 0) indexCurrentSoundscape = soundscapes.size() - 1;
			break;
		case ' ':
			ticking = !ticking;
			break;
		case 'x':
			ms_soundscape_play_sound(soundscapes[indexCurrentSoundscape]);
			break;
	}
}
