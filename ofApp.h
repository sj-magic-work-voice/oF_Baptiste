/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "ofMain.h"
#include "ofxSyphon.h"

#include "sj_common.h"
#include "th_fft.h"


/************************************************************
************************************************************/

/**************************************************
**************************************************/
struct AUDIO_SAMPLE : private Noncopyable{
	vector<float> Left;
	vector<float> Right;
	
	void resize(int size){
		/*
		Left.resize(size);
		Right.resize(size);
		*/
		
		Left.assign(size, 0.0);
		Right.assign(size, 0.0);
	}
};

/**************************************************
**************************************************/
struct VBO_SET : private Noncopyable{
	ofVbo Vbo;
	vector<ofVec3f> VboVerts;
	vector<ofFloatColor> VboColor;
	
	void setup(int size){
		VboVerts.resize(size);
		VboColor.resize(size);
		
		Vbo.setVertexData(&VboVerts[0], VboVerts.size(), GL_DYNAMIC_DRAW);
		Vbo.setColorData(&VboColor[0], VboColor.size(), GL_DYNAMIC_DRAW);
	}
	
	void set_singleColor(const ofColor& color){
		for(int i = 0; i < VboColor.size(); i++) { VboColor[i].set( double(color.r)/255, double(color.g)/255, double(color.b)/255, double(color.a)/255); }
	}
	
	void set_Color(int id, int NumPerId, const ofColor& color){
		for(int i = 0; i < NumPerId; i++){
			VboColor[id * NumPerId + i].set( double(color.r)/255, double(color.g)/255, double(color.b)/255, double(color.a)/255);
		}
	}
	
	void update(){
		Vbo.updateVertexData(&VboVerts[0], VboVerts.size());
		Vbo.updateColorData(&VboColor[0], VboColor.size());
	}
	
	void draw(int drawMode){
		Vbo.draw(drawMode, 0, VboVerts.size());
	}
	
	void draw(int drawMode, int total){
		if(VboVerts.size() < total) total = VboVerts.size();
		Vbo.draw(drawMode, 0, total);
	}
};

/**************************************************
**************************************************/
class ofApp : public ofBaseApp{
private:
	/****************************************
	****************************************/
	enum{
		FONT_S,
		FONT_M,
		FONT_L,
		
		NUM_FONTSIZE,
	};
	
	enum{
		GRAPH__L,
		GRAPH__M,
		GRAPH__H,
		
		GRAPH__L_FILLED,
		GRAPH__M_FILLED,
		GRAPH__H_FILLED,
		
		GRAPH__MIXED,
		GRAPH__CONTOUR,
		
		GRAPH__AND,
		GRAPH__OR,
		GRAPH__OR_AND,
		
		GRAPH__FFT,
		
		NUM_GRAPHS,
	};
	
	enum STATE_CONTENTS{
		CONTENTS_WHOLE,
		CONTENTS_ONE,
	};
	
	enum{
		NUM_FREQS_L = 2,
		NUM_FREQS_M = 3,
		NUM_FREQS_H = 4,
	};
	
	/****************************************
	****************************************/
	ofFbo fbo[NUM_GRAPHS];
	ofPoint Fbo_DispPos[NUM_GRAPHS] = {
		ofPoint(40, 40),
		ofPoint(40, 260),
		ofPoint(40, 480),
		
		ofPoint(740, 40),
		ofPoint(740, 260),
		ofPoint(740, 480),
		
		ofPoint(390, 480),
		ofPoint(390, 260),
		
		ofPoint(1090, 40),
		ofPoint(1440, 40),
		ofPoint(1120, 260),
		
		ofPoint(1120, 480),
	};
	ofVec2f Fbo_DispSize[NUM_GRAPHS] = {
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),

		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),

		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),

		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH, FBO_LIST_HEIGHT),
		ofVec2f(FBO_LIST_WIDTH * 2, FBO_LIST_HEIGHT),

		ofVec2f(FBO_LIST_WIDTH * 2, FBO_LIST_HEIGHT),
	};
	
	STATE_CONTENTS StateContents;
	int ActiveContents_id;
	float t_ContentsChange;
	const float d_ActiveContents_Transition;
	
	
	VBO_SET Vboset_L[NUM_FREQS_L];
	VBO_SET Vboset_M[NUM_FREQS_M];
	VBO_SET Vboset_H[NUM_FREQS_H];
	
	VBO_SET Vboset_L_Filled[NUM_FREQS_L];
	VBO_SET Vboset_M_Filled[NUM_FREQS_M];
	VBO_SET Vboset_H_Filled[NUM_FREQS_H];
	
	int FreqIds_L[NUM_FREQS_L];// = {2};
	int FreqIds_M[NUM_FREQS_M];// = {10, 11};
	int FreqIds_H[NUM_FREQS_H];// = {20, 21, 22};
	
	/********************
	********************/
	bool b_DispGui;
	bool b_DispFrameRate;
	
	/********************
	********************/
	ofSoundStream soundStream;
	int soundStream_Input_DeviceId;
	int soundStream_Output_DeviceId;
	
	AUDIO_SAMPLE AudioSample;
	
	ofTrueTypeFont font[NUM_FONTSIZE];
	
	THREAD_FFT* fft_thread;
	VBO_SET Vboset_fft;
	
	/********************
	********************/
	ofShader shader_Or;
	ofShader shader_And;
	ofShader shader_Or_And;
	
	ofShader shader_contour;
	
	/********************
	********************/
	int png_id;
	
	/****************************************
	****************************************/
	void setup_Gui();
	void Clear_fbo(ofFbo& fbo);
	void Refresh_FFTVerts();
	void Reset_SoundStream();
	void drawFbo_FFT();
	void drawFbo_toScreen(ofFbo& _fbo, const ofPoint& Coord_zero, const int Width, const int Height);
	void Refresh_VboVerts_LMH(int FreqIds[], int NumFreqIds, VBO_SET Vboset[], VBO_SET Vboset_Filled[]);
	void drawFbo_LMH_each(ofFbo& _fbo, VBO_SET Vboset[], int NumFreqIds, int drawMode, int LineWidth);
	void drawFbo_LMH_mixed(int LineWidth);
	void update_LMH_Range();
	
	void drawFbo_Or();
	void drawFbo_and();
	void drawFbo_Or_And();
	void drawFbo_Contour();
	
	void draw_ConnectionLine();
	void draw_Fbo_id();
	
	void draw_Fbo_Except(int ExceptId);
	void draw_AllFbo();
	void draw_Fbo_inTransition(float now);
	void Change_ActiveContents(int _ActiveContents_id);

	
public:
	/****************************************
	****************************************/
	ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId);
	~ofApp();
	
	void exit();
	
	void setup();
	void update();
	void draw();
	
	void audioIn(ofSoundBuffer & buffer);
	void audioOut(ofSoundBuffer & buffer);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
};
