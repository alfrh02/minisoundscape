#include "ofApp.h"

#define MA_ENGINE_MAX_LISTENERS 1

#include "minisoundscape.h"

#define PATH(x) (ofToDataPath(x, true))

using namespace glm;

static ma_engine engine;
static ms_soundscape soundscape;
static ms_sound_speaker speaker;

ofTexture loadIcon(const std::string& filepath) {
	ofImage img;
	img.load(filepath);
	img.mirror(true, false);
	ofTexture tex = img.getTexture();
	tex.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST); // set texture filtering to nearest as to preserve pixel art
	return tex;
}

ofMesh makeQuad(const float size = 0.5) {
	ofMesh mesh;

	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	mesh.addVertex(vec3(-size, 0, -size));
	mesh.addVertex(vec3( size, 0, -size));
	mesh.addVertex(vec3( size, 0,  size));
	mesh.addVertex(vec3(-size, 0,  size));

	mesh.addTriangle(0, 1, 2);
	mesh.addTriangle(0, 2, 3);

	return mesh;
}

vec3 getGridCoords(const vec3 pos) {
	return vec3(pos.x + (GAP * pos.x), pos.y, pos.z + (GAP * pos.z));
}

//--------------------------------------------------------------
ofApp::~ofApp() {
	ms_soundscape_uninit(&soundscape);
}

//--------------------------------------------------------------
void ofApp::setup(){
	/* --- camera setup --- */
	cam.setPosition(vec3(8.0, 8.0, 8.0 * 1.5));
	cam.setTarget(vec3(0, 1.0, 0));
	cam.setNearClip(0.05);
	cam.setFarClip(200.0);
	cam.setFov(30);

	/* --- scene setup --- */
	ofEnableDepthTest();
	ofEnableNormalizedTexCoords();
	ofDisableArbTex();
	ofSetFrameRate(144);

	std::string v = R"(
		#version 150

		uniform mat4 modelViewProjectionMatrix;
		uniform float spriteRadius;
		uniform mat4 cameraMatrix;

		in vec4 position;

		out vec2 spriteCoord;

		void main(){
			vec4 pos = position;

			switch(gl_VertexID % 4) {
				case 0: spriteCoord = vec2(-1.0, -1.0); break;
				case 1: spriteCoord = vec2( 1.0, -1.0); break;
				case 2: spriteCoord = vec2( 1.0,  1.0); break;
				case 3: spriteCoord = vec2(-1.0,  1.0); break;
			}

			vec4 offset = vec4(spriteCoord * spriteRadius, 0.0, 0.0);
			offset = cameraMatrix * offset;

			pos += offset;
			gl_Position = modelViewProjectionMatrix * pos;
		}
	)";

	std::string f = R"(
		#version 150

		uniform sampler2D tex;
		uniform vec4 spriteColor;
		uniform bool tapering;

		in vec2 spriteCoord;

		out vec4 fragColor;

		void main(){
			vec3 col = texture(tex, (spriteCoord + 1) / 2).rgb;
			if (col == vec3(0.0, 0.0, 0.0)) discard;

			if (tapering) {
				float rsqr = dot(spriteCoord, spriteCoord);
				if (rsqr  > 1.0) discard;
				float a = 1.0 - rsqr;
				float w = 0.5;
				float wsqr = w * w;
				a *= wsqr / (wsqr + rsqr);
				col *= a;
			}

			col *= spriteColor.rgb;

			fragColor = vec4(col, spriteColor.w);
		}
	)";

	spriteShader.setupShaderFromSource(GL_VERTEX_SHADER,   v.c_str());
	spriteShader.setupShaderFromSource(GL_FRAGMENT_SHADER, f.c_str());
	spriteShader.bindDefaults();
	spriteShader.linkProgram();

	mTile       = makeQuad(0.5);
	mMicrophone = makeQuad(0.0);
	mSpeaker    = makeQuad(0.0);

	texMicrophone = loadIcon("microphone.png");
	texSpeaker    = loadIcon("speaker.png");

	/* --- audio setup --- */
	ma_result result = ma_engine_init(NULL, &engine);

	ms_soundscape_init("soundscape", &engine, "", &soundscape);

	ms_sound_speaker_init("speaker", &speaker, -5, 0, 0);

	ofxXmlSettings settings;
	settings.load("settings.xml");
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

		ms_sound_add_speaker(sound, &speaker);

		ms_soundscape_add_sound(&soundscape, sound);
		settings.popTag();
	}
	settings.popTag(); // sounds tag

	ms_soundscape_start(&soundscape);
}

//--------------------------------------------------------------
void ofApp::update(){
	if (ticking) ms_soundscape_tick(&soundscape);
}

//--------------------------------------------------------------
void ofApp::draw(){
	stringstream str;
	str << "Press space to toggle ticking" << endl;
	str << "Press x to play a sound" << endl;
	str << "Ticking: " << to_string(ticking) << endl;
	ofDrawBitmapString(str.str(), 8, 16);

	cam.begin();
	ofPushMatrix();
		ofTranslate(getGridCoords(-vec3(WIDTH / 2, 0, HEIGHT / 2)) + vec3(0.5));
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		spriteShader.begin();
			spriteShader.setUniformMatrix4f("cameraMatrix", cam.getLocalTransformMatrix());
			spriteShader.setUniform1i("tapering",     false);
			spriteShader.setUniform1f("spriteRadius", 0.1);
			spriteShader.setUniform4f("spriteColor",  vec4(1.0, 0.0, 0.0, 1.0));
			spriteShader.setUniformTexture("tex",     texMicrophone, 0);

			mMicrophone.draw(); // draws at 0, 0, 0

			spriteShader.setUniformTexture("tex", texSpeaker, 0);
			ofPushMatrix();
				ofTranslate(getGridCoords(vec3(speaker.x, speaker.y, speaker.z)));
				mSpeaker.draw();
			ofPopMatrix();
		spriteShader.end();
		ofDisableBlendMode();
		for (float y = 0; y < HEIGHT; y++) {
			for (float x = 0; x < WIDTH; x++) {
				ofPushMatrix();
					ofTranslate(getGridCoords(vec3(x, 0, y)));
					mTile.drawWireframe();
				ofPopMatrix();
			}
		}
	ofPopMatrix();
	cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch(key) {
		case 'w': posSpeaker.z -= 1.0; break;
		case 'a': posSpeaker.x -= 1.0; break;
		case 's': posSpeaker.z += 1.0; break;
		case 'd': posSpeaker.x += 1.0; break;

		case ' ':
			ticking = !ticking;
			break;
		case 'x':
			ms_soundscape_play_sound_skip_empty(&soundscape);
			break;
	}

	if	    (posSpeaker.z < 0.0) posSpeaker.z = HEIGHT - 1;
	else if (posSpeaker.z > HEIGHT - 1) posSpeaker.z = 0.0;

	if	    (posSpeaker.x < 0.0) posSpeaker.x = WIDTH - 1;
	else if (posSpeaker.x > WIDTH - 1) posSpeaker.x = 0.0;

	speaker.x = posSpeaker.x;
	speaker.y = posSpeaker.y;
	speaker.z = posSpeaker.z;

}