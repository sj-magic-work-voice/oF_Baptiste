/************************************************************
************************************************************/
#pragma once

/************************************************************
************************************************************/
#include "stdio.h"

#include "ofMain.h"
#include "ofxGui.h"

/************************************************************
************************************************************/
#define ERROR_MSG(); printf("Error in %s:%d\n", __FILE__, __LINE__);

/************************************************************
************************************************************/
enum{
	WINDOW_WIDTH	= 1800,	// 切れの良い解像度でないと、ofSaveScreen()での画面保存が上手く行かなかった(真っ暗な画面が保存されるだけ).
	WINDOW_HEIGHT	= 700,
};

enum{
	/*
	FBO_WIDTH	= 640,
	FBO_HEIGHT	= 360,
	*/
	FBO_WIDTH	= 1280,
	FBO_HEIGHT	= 720,
};

enum{
	FBO_LIST_WIDTH	= 320,
	FBO_LIST_HEIGHT	= 180,
};

enum{
	BUF_SIZE_S = 500,
	BUF_SIZE_M = 1000,
	BUF_SIZE_L = 6000,
};

enum{
	AUDIO_BUF_SIZE = 512,
	
	AUDIO_BUFFERS = 2,
	AUDIO_SAMPLERATE = 44100,
};

enum{
	GRAPH_BAR_WIDTH__FFT_GAIN = 4,
	GRAPH_BAR_SPACE__FFT_GAIN = 6,
};

enum{
	GRAPH__L,
	GRAPH__M,
	GRAPH__H,
	
	GRAPH__L_FILLED,
	GRAPH__M_FILLED,
	GRAPH__H_FILLED,
	
	GRAPH__MIXED,
	GRAPH__MIXED_TOP_VIEW,
	
	GRAPH__AND,
	GRAPH__OR,
	GRAPH__OR_AND,
	
	GRAPH__FFT,
	
	NUM_GRAPHS,
};
	


/************************************************************
************************************************************/

/**************************************************
Derivation
	class MyClass : private Noncopyable {
	private:
	public:
	};
**************************************************/
class Noncopyable{
protected:
	Noncopyable() {}
	~Noncopyable() {}

private:
	void operator =(const Noncopyable& src);
	Noncopyable(const Noncopyable& src);
};


/**************************************************
**************************************************/
class GUI_GLOBAL{
private:
	/****************************************
	****************************************/
	
public:
	/****************************************
	****************************************/
	void setup(string GuiName, string FileName = "gui.xml", float x = 10, float y = 10);
	
	ofxGuiGroup Group_Graph;
		ofxFloatSlider LPFAlpha_dt__FFTGain;
		ofxFloatSlider LPFAlpha_dt__FFTPhase;
		
		ofxFloatSlider Val_DispMax__FFTGain;
		ofxFloatSlider Val_DispMax__Sin;
		
	ofxGuiGroup Group_LMH;
		ofxIntSlider Id_Low;
		ofxIntSlider Id_Middle;
		ofxIntSlider Id_High;
		
	ofxGuiGroup Group_Overlay;
		ofxToggle b_Do_Overlay;
		ofxFloatSlider Overlay_Alpha;
		
		ofxToggle b_Overlay_L;
		ofxToggle b_Overlay_M;
		ofxToggle b_Overlay_H;
		
	
	/****************************************
	****************************************/
	ofxPanel gui;
};

/************************************************************
************************************************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt);
double LPF(double LastVal, double CurrentVal, double Alpha);
double sj_max(double a, double b);

/************************************************************
************************************************************/
extern GUI_GLOBAL* Gui_Global;

extern FILE* fp_Log;
extern FILE* fp_Log_main;
extern FILE* fp_Log_Audio;
extern FILE* fp_Log_fft;

extern int GPIO_0;
extern int GPIO_1;


/************************************************************
************************************************************/

