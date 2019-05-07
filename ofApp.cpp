/************************************************************
************************************************************/
#include "ofApp.h"

/************************************************************
************************************************************/

/******************************
******************************/
ofApp::ofApp(int _soundStream_Input_DeviceId, int _soundStream_Output_DeviceId)
: fft_thread(THREAD_FFT::getInstance())
, soundStream_Input_DeviceId(_soundStream_Input_DeviceId)
, soundStream_Output_DeviceId(_soundStream_Output_DeviceId)
, b_DispGui(true)
, b_DispFrameRate(false)
, png_id(0)
, ActiveContents_id(-1)
, StateContents(CONTENTS_WHOLE)
, t_ContentsChange(0)
, d_ActiveContents_Transition(0.5)
{
	/********************
	********************/
	font[FONT_S].load("RictyDiminished-Regular.ttf", 10, true, true, true);
	font[FONT_M].load("RictyDiminished-Regular.ttf", 12, true, true, true);
	font[FONT_L].load("RictyDiminished-Regular.ttf", 30, true, true, true);
	
	/********************
	********************/
	fp_Log			= fopen("../../../data/Log.csv", "w");
	fp_Log_main		= fopen("../../../data/Log_main.csv", "w");
	fp_Log_Audio 	= fopen("../../../data/Log_Audio.csv", "w");
	fp_Log_fft 		= fopen("../../../data/Log_fft.csv", "w");
}

/******************************
******************************/
ofApp::~ofApp()
{
	if(fp_Log)			fclose(fp_Log);
	if(fp_Log_main)		fclose(fp_Log_main);
	if(fp_Log_Audio)	fclose(fp_Log_Audio);
	if(fp_Log_fft)		fclose(fp_Log_fft);
}

/******************************
******************************/
void ofApp::exit(){
	/********************
	ofAppとaudioが別threadなので、ここで止めておくのが安全.
	********************/
	soundStream.stop();
	soundStream.close();
	
	/********************
	********************/
	fft_thread->exit();
	try{
		/********************
		stop済みのthreadをさらにstopすると、Errorが出るようだ。
		********************/
		while(fft_thread->isThreadRunning()){
			fft_thread->waitForThread(true);
		}
		
	}catch(...){
		printf("Thread exiting Error\n");
	}
	
	/********************
	********************/
	printf("\n> Good bye\n");
}	

/******************************
******************************/
void ofApp::update_LMH_Range(){
	/********************
	********************/
	for(int i = 0; i < NUM_FREQS_L; i++) { FreqIds_L[i] = Gui_Global->Id_Low + i; }
	for(int i = 0; i < NUM_FREQS_M; i++) { FreqIds_M[i] = Gui_Global->Id_Middle + i; }
	for(int i = 0; i < NUM_FREQS_H; i++) { FreqIds_H[i] = Gui_Global->Id_High + i; }
	
	/********************
	********************/
	Vboset_fft.set_singleColor(ofColor(255, 255, 255, 100));
	
	for(int i = 0; i < NUM_FREQS_L; i++) { Vboset_fft.set_Color(FreqIds_L[i], 4, ofColor(255, 0, 0, 100)); }
	for(int i = 0; i < NUM_FREQS_M; i++) { Vboset_fft.set_Color(FreqIds_M[i], 4, ofColor(255, 255, 0, 100)); }
	for(int i = 0; i < NUM_FREQS_H; i++) { Vboset_fft.set_Color(FreqIds_H[i], 4, ofColor(0, 255, 0, 100)); }
	
}

/******************************
******************************/
void ofApp::setup(){
	/********************
	********************/
	ofSetWindowTitle("Baptiste");
	
	ofSetWindowShape( WINDOW_WIDTH, WINDOW_HEIGHT );
	ofSetVerticalSync(true);
	ofSetFrameRate(30);
	ofSetEscapeQuitsApp(false);
	
	/********************
	********************/
	setup_Gui();
	
	/********************
	********************/
	Vboset_fft.setup(AUDIO_BUF_SIZE/2 * 4); /* square */
	update_LMH_Range();
	
	/* */
	{
		const int Alpha = 110;
		for(int i = 0; i < NUM_FREQS_L; i++) { Vboset_L[i].setup(FBO_WIDTH); Vboset_L[i].set_singleColor(ofColor(255, 0, 0, Alpha)); }
		for(int i = 0; i < NUM_FREQS_M; i++) { Vboset_M[i].setup(FBO_WIDTH); Vboset_M[i].set_singleColor(ofColor(255, 255, 0, Alpha)); }
		for(int i = 0; i < NUM_FREQS_H; i++) { Vboset_H[i].setup(FBO_WIDTH); Vboset_H[i].set_singleColor(ofColor(0, 255, 0, Alpha)); }
		
		for(int i = 0; i < NUM_FREQS_L; i++) { Vboset_L_Filled[i].setup(FBO_WIDTH * 2); Vboset_L_Filled[i].set_singleColor(ofColor(255, 255, 255, Alpha)); }
		for(int i = 0; i < NUM_FREQS_M; i++) { Vboset_M_Filled[i].setup(FBO_WIDTH * 2); Vboset_M_Filled[i].set_singleColor(ofColor(255, 255, 255, Alpha)); }
		for(int i = 0; i < NUM_FREQS_H; i++) { Vboset_H_Filled[i].setup(FBO_WIDTH * 2); Vboset_H_Filled[i].set_singleColor(ofColor(255, 255, 255, Alpha)); }
	}
	
	/********************
	********************/
	for(int i = 0; i < NUM_GRAPHS; i++) { fbo[i].allocate(FBO_WIDTH, FBO_HEIGHT, GL_RGBA); Clear_fbo(fbo[i]); }
	
	/********************
	********************/
	AudioSample.resize(AUDIO_BUF_SIZE);

	fft_thread->setup();
	
	/********************
	********************/
	Refresh_FFTVerts();
	Refresh_VboVerts_LMH(FreqIds_L, NUM_FREQS_L, Vboset_L, Vboset_L_Filled);
	Refresh_VboVerts_LMH(FreqIds_M, NUM_FREQS_M, Vboset_M, Vboset_M_Filled);
	Refresh_VboVerts_LMH(FreqIds_H, NUM_FREQS_H, Vboset_H, Vboset_H_Filled);
	
	/********************
	********************/
	shader_Or.load( "shader/Or.vert", "shader/Or.frag" );
	shader_And.load( "shader/and.vert", "shader/and.frag" );
	shader_Or_And.load( "shader/Or_And.vert", "shader/Or_And.frag" );
	shader_contour.load( "shader/Contour.vert", "shader/Contour.frag" );
	
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	soundStream.printDeviceList();
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	Reset_SoundStream();
}

/******************************
******************************/
void ofApp::Refresh_FFTVerts()
{
	/********************
	********************/
	float BarWidth = GRAPH_BAR_WIDTH__FFT_GAIN;
	float BarSpace = GRAPH_BAR_SPACE__FFT_GAIN;
	
	/********************
	********************/
	float Gui_DispGain = Gui_Global->Val_DispMax__FFTGain;
	for(int j = 0; j < AUDIO_BUF_SIZE/2; j++){
		Vboset_fft.VboVerts[j * 4 + 0].set( BarSpace * j            , 0 );
		Vboset_fft.VboVerts[j * 4 + 1].set( BarSpace * j            , fft_thread->getArrayVal_x_DispGain(j, Gui_DispGain, FBO_HEIGHT, true) );
		Vboset_fft.VboVerts[j * 4 + 2].set( BarSpace * j  + BarWidth, fft_thread->getArrayVal_x_DispGain(j, Gui_DispGain, FBO_HEIGHT, true) );
		Vboset_fft.VboVerts[j * 4 + 3].set( BarSpace * j  + BarWidth, 0 );
	}
}

/******************************
******************************/
void ofApp::Refresh_VboVerts_LMH(int FreqIds[], int NumFreqIds, VBO_SET Vboset[], VBO_SET Vboset_Filled[])
{
	int dt = (int) ((1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE) / FBO_WIDTH);
	
	for(int i = 0; i < NumFreqIds; i++){
		double f = fft_thread->getFreq(FreqIds[i]);
		double Amp = fft_thread->getArrayVal_x_DispGain(FreqIds[i], Gui_Global->Val_DispMax__Sin, FBO_HEIGHT/2, false);
		
		double Phase = fft_thread->getPhase(FreqIds[i]);
		Phase = ofMap(Phase, 0, 360, 0, 1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE, true);
		
		for(int _x = 0; _x < FBO_WIDTH; _x++){
			double _y = Amp * cos(TWO_PI * f * ((dt * _x + Phase) / 1e+6));
			
			Vboset[i].VboVerts[_x].set(_x, _y);
			
			Vboset_Filled[i].VboVerts[_x * 2 + 0].set(_x, _y);
			Vboset_Filled[i].VboVerts[_x * 2 + 1].set(_x, -FBO_HEIGHT/2);
		}
	}
}

/******************************
******************************/
void ofApp::Reset_SoundStream()
{
	/********************
	settings.setInListener(this);
	settings.setOutListener(this);
	settings.sampleRate = 44100;
	settings.numInputChannels = 2;
	settings.numOutputChannels = 2;
	settings.bufferSize = bufferSize;
	
	soundStream.setup(settings);
	********************/
	ofSoundStreamSettings settings;
	
	if( soundStream_Input_DeviceId == -1 ){
		ofExit();
		return;
		
	}else{
		vector<ofSoundDevice> devices = soundStream.getDeviceList();
		
		if( soundStream_Input_DeviceId != -1 ){
			settings.setInDevice(devices[soundStream_Input_DeviceId]);
			settings.setInListener(this);
			settings.numInputChannels = AUDIO_BUFFERS;
		}else{
			settings.numInputChannels = 0;
		}
		
		if( soundStream_Output_DeviceId != -1 ){
			if(devices[soundStream_Output_DeviceId].name == "Apple Inc.: Built-in Output"){
				printf("!!!!! prohibited to use [%s] for output ... by SJ for safety !!!!!\n", devices[soundStream_Output_DeviceId].name.c_str());
				fflush(stdout);
				
				settings.numOutputChannels = 0;
				
			}else{
				settings.setOutDevice(devices[soundStream_Output_DeviceId]);
				settings.numOutputChannels = AUDIO_BUFFERS;
				settings.setOutListener(this); /* Don't forget this */
			}
		}else{
			settings.numOutputChannels = 0;
		}
		
		settings.numBuffers = 4;
		settings.sampleRate = AUDIO_SAMPLERATE;
		settings.bufferSize = AUDIO_BUF_SIZE;
	}
	
	/********************
	soundStream.setup()の位置に注意:最後
		setup直後、audioIn()/audioOut()がstartする.
		これらのmethodは、fft_threadにaccessするので、start前にReStart()によって、fft_threadが初期化されていないと、不正accessが発生してしまう.
	********************/
	soundStream.setup(settings);
	// soundStream.start();
}

/******************************
description
	memoryを確保は、app start後にしないと、
	segmentation faultになってしまった。
******************************/
void ofApp::setup_Gui()
{
	/********************
	********************/
	Gui_Global = new GUI_GLOBAL;
	Gui_Global->setup("Baptiste", "gui.xml", 1000, 10);
}

/******************************
******************************/
void ofApp::Clear_fbo(ofFbo& fbo)
{
	fbo.begin();
	
	// Clear with alpha, so we can capture via syphon and composite elsewhere should we want.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	ofClear(0, 0, 0, 0);
	
	fbo.end();
}

/******************************
******************************/
void ofApp::update(){

	fft_thread->update();
	update_LMH_Range();
}

/******************************
******************************/
void ofApp::draw(){
	/********************
	********************/
	float now = ofGetElapsedTimef();
	
	/********************
	********************/
	ofClear(0, 0, 0, 255);
	ofSetColor(255, 255, 255, 255);
	
	/********************
	********************/
	Refresh_FFTVerts();
	Vboset_fft.update(); // audioOutからの呼び出しだと segmentation fault となってしまった.
	
	Refresh_VboVerts_LMH(FreqIds_L, NUM_FREQS_L, Vboset_L, Vboset_L_Filled);
	for(int i = 0; i < NUM_FREQS_L; i++) {Vboset_L[i].update(); Vboset_L_Filled[i].update();}
	
	Refresh_VboVerts_LMH(FreqIds_M, NUM_FREQS_M, Vboset_M, Vboset_M_Filled);
	for(int i = 0; i < NUM_FREQS_M; i++) {Vboset_M[i].update(); Vboset_M_Filled[i].update();}
	
	Refresh_VboVerts_LMH(FreqIds_H, NUM_FREQS_H, Vboset_H, Vboset_H_Filled);
	for(int i = 0; i < NUM_FREQS_H; i++) {Vboset_H[i].update(); Vboset_H_Filled[i].update();}
	
	
	/********************
	********************/
	/* */
	drawFbo_FFT();
	
	/* */
	const int LineWidth = int(1.5 * FBO_WIDTH/FBO_LIST_WIDTH);
	drawFbo_LMH_each(fbo[GRAPH__L], Vboset_L, NUM_FREQS_L, GL_LINE_STRIP, LineWidth);
	drawFbo_LMH_each(fbo[GRAPH__M], Vboset_M, NUM_FREQS_M, GL_LINE_STRIP, LineWidth);
	drawFbo_LMH_each(fbo[GRAPH__H], Vboset_H, NUM_FREQS_H, GL_LINE_STRIP, LineWidth);
	
	drawFbo_LMH_each(fbo[GRAPH__L_FILLED], Vboset_L_Filled, NUM_FREQS_L, GL_TRIANGLE_STRIP, LineWidth);
	drawFbo_LMH_each(fbo[GRAPH__M_FILLED], Vboset_M_Filled, NUM_FREQS_M, GL_TRIANGLE_STRIP, LineWidth);
	drawFbo_LMH_each(fbo[GRAPH__H_FILLED], Vboset_H_Filled, NUM_FREQS_H, GL_TRIANGLE_STRIP, LineWidth);
	
	drawFbo_LMH_mixed(LineWidth);
	
	/* */
	drawFbo_Contour();
	
	/* */
	drawFbo_Or();
	drawFbo_and();
	drawFbo_Or_And();
	
	/********************
	********************/
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	switch(StateContents){
		case CONTENTS_WHOLE:
			draw_AllFbo();
			break;
			
		case CONTENTS_ONE:
			if( d_ActiveContents_Transition < now - t_ContentsChange ){
				drawFbo_toScreen(fbo[ActiveContents_id], ofPoint(0, 0), ofGetWidth(), ofGetHeight());
			}else{
				draw_Fbo_Except(ActiveContents_id);
				draw_Fbo_inTransition(now);
			}
			break;
	}
	
	/********************
	********************/
	ofEnableAlphaBlending();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	// ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	if(Gui_Global->b_Do_Overlay)	drawFbo_toScreen(fbo[GRAPH__CONTOUR], ofPoint(0, 0), ofGetWidth(), ofGetHeight());
	
	/********************
	********************/
	if(b_DispGui) Gui_Global->gui.draw();
	
	if(b_DispFrameRate){
		ofSetColor(255, 0, 0, 255);
		
		char buf[BUF_SIZE_S];
		sprintf(buf, "%5.1f", ofGetFrameRate());
		
		font[FONT_M].drawString(buf, 30, 30);
	}
}

/******************************
******************************/
void ofApp::draw_Fbo_Except(int ExceptId){
	for(int i = 0; i < NUM_GRAPHS; i++){
		if(i != ExceptId)	drawFbo_toScreen(fbo[i], Fbo_DispPos[i], Fbo_DispSize[i].x, Fbo_DispSize[i].y);
	}
	
	draw_ConnectionLine();
	draw_Fbo_id();
}

/******************************
******************************/
void ofApp::draw_AllFbo(){
	draw_Fbo_Except(-1);
}

/******************************
******************************/
void ofApp::draw_Fbo_inTransition(float now){
	float ratio = (now - t_ContentsChange) / d_ActiveContents_Transition;
	
	ofPoint pos;
	float from = Fbo_DispPos[ActiveContents_id].x;
	float to = 0;
	pos.x = from + ratio * (to - from);
	
	from = Fbo_DispPos[ActiveContents_id].y;
	to = 0;
	pos.y = from + ratio * (to - from);
	
	ofVec2f Size;
	from = Fbo_DispSize[ActiveContents_id].x;
	to = ofGetWidth();
	Size.x = from + ratio * (to - from);
	
	from = Fbo_DispSize[ActiveContents_id].y;
	to = ofGetHeight();
	Size.y = from + ratio * (to - from);
	
	drawFbo_toScreen(fbo[ActiveContents_id], pos, Size.x, Size.y);
}

/******************************
******************************/
void ofApp::draw_ConnectionLine(){
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	ofSetColor(255, 255, 255, 140);
	glPointSize(1.0);
	glLineWidth(1);
	ofNoFill();
	
	ofDrawLine(360, 130, 375, 130);
	ofDrawLine(360, 350, 375, 350);
	ofDrawLine(360, 570, 390, 570);
	ofDrawLine(375, 130, 375, 570);
	
	ofDrawLine(530, 440, 530, 480);
	ofDrawLine(550, 440, 550, 480);
	ofDrawLine(570, 440, 570, 480);
	
	ofDrawLine(725, 130, 740, 130);
	ofDrawLine(725, 350, 740, 350);
	ofDrawLine(710, 570, 740, 570);
	ofDrawLine(725, 130, 725, 570);
	
	ofDrawLine(1060, 130, 1090, 130);
	ofDrawLine(1060, 350, 1075, 350);
	ofDrawLine(1060, 570, 1075, 570);
	ofDrawLine(1075, 130, 1075, 570);
	
	ofDrawLine(1410, 130, 1440, 130);
	ofDrawLine(1425, 130, 1425, 260);
}

/******************************
******************************/
void ofApp::draw_Fbo_id(){
	ofEnableAlphaBlending();
	// ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	ofSetColor(255, 255, 255, 100);
	glPointSize(1.0);
	glLineWidth(1);
	ofNoFill();
	
	for(int i = 0; i < NUM_GRAPHS; i++){
		char buf[BUF_SIZE_S];
		sprintf(buf, "%X", i);
		
		font[FONT_S].drawString(buf, Fbo_DispPos[i].x, Fbo_DispPos[i].y);
	}
}

/******************************
******************************/
void ofApp::drawFbo_toScreen(ofFbo& _fbo, const ofPoint& Coord_zero, const int Width, const int Height)
{
	ofPushStyle();
	ofPushMatrix();
		/********************
		********************/
		ofTranslate(Coord_zero);
		
		_fbo.draw(0, 0, Width, Height);
		
	ofPopMatrix();
	ofPopStyle();
}

/******************************
******************************/
void ofApp::drawFbo_FFT()
{
 	float Screen_y_max = FBO_HEIGHT;
	float Val_Disp_y_Max = Gui_Global->Val_DispMax__FFTGain;
	float Screen_Disp_Sin_max =  ofMap(Gui_Global->Val_DispMax__Sin, 0, Gui_Global->Val_DispMax__FFTGain, 0, FBO_HEIGHT, true);
	
	/********************
	********************/
	fbo[GRAPH__FFT].begin();
		
		ofDisableAlphaBlending();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		ofPushStyle();
		ofPushMatrix();
			/********************
			********************/
			ofTranslate(0, fbo[GRAPH__FFT].getHeight() - 1);
			ofScale(1, -1, 1);
			
			/********************
			y目盛り
			********************/
			if(0 < Val_Disp_y_Max){
				const int num_lines = 5;
				const double y_step = Screen_y_max/num_lines;
				for(int i = 0; i < num_lines; i++){
					int y = int(i * y_step + 0.5);
					
					ofSetColor(ofColor(50));
					ofSetLineWidth(1 * FBO_WIDTH/FBO_LIST_WIDTH);
					ofNoFill();
					ofDrawLine(0, y, FBO_WIDTH - 1, y);
		
					/********************
					********************/
					char buf[BUF_SIZE_S];
					sprintf(buf, "%7.4f", Val_Disp_y_Max/num_lines * i);
					
					ofSetColor(ofColor(200));
					ofScale(1, -1, 1); // 文字が上下逆さまになってしまうので.
					font[FONT_L].drawString(buf, FBO_WIDTH - 1 - font[FONT_L].stringWidth(buf) - 10, -y); // y posはマイナス
					ofScale(1, -1, 1); // 戻す.
				}
			}
			
			/********************
			********************/
			ofSetColor(255);
			glPointSize(1.0);
			glLineWidth(1);
			ofFill();
			
			Vboset_fft.draw(GL_QUADS);
			
			/********************
			********************/
			ofSetColor(255, 255, 0, 150);
			glPointSize(1.0);
			glLineWidth(1 * FBO_WIDTH/FBO_LIST_WIDTH);
			
			ofDrawLine(0, Screen_Disp_Sin_max, FBO_WIDTH, Screen_Disp_Sin_max);
			
		ofPopMatrix();
		ofPopStyle();
	fbo[GRAPH__FFT].end();
}

/******************************
******************************/
void ofApp::drawFbo_LMH_each(ofFbo& _fbo, VBO_SET Vboset[], int NumFreqIds, int drawMode, int LineWidth)
{
	/********************
	********************/
	_fbo.begin();
		
		ofDisableAlphaBlending();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		ofPushStyle();
		ofPushMatrix();
			/********************
			********************/
			ofTranslate(0, _fbo.getHeight()/2);
			ofScale(1, -1, 1);
			
			/********************
			********************/
			ofSetColor(255);
			glPointSize(1.0);
			glLineWidth(LineWidth);
			ofFill();
			
			for(int i = 0; i < NumFreqIds; i++) Vboset[i].draw(drawMode);
			
		ofPopMatrix();
		ofPopStyle();
	_fbo.end();
}

/******************************
******************************/
void ofApp::drawFbo_LMH_mixed(int LineWidth)
{
	/********************
	********************/
	fbo[GRAPH__MIXED].begin();
		
		ofDisableAlphaBlending();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		ofPushStyle();
		ofPushMatrix();
			/********************
			********************/
			ofTranslate(0, fbo[GRAPH__MIXED].getHeight()/2);
			ofScale(1, -1, 1);
			
			/********************
			********************/
			ofSetColor(255);
			glPointSize(1.0);
			glLineWidth(LineWidth);
			ofFill();
			
			for(int i = 0; i < NUM_FREQS_L; i++) Vboset_L[i].draw(GL_LINE_STRIP);
			for(int i = 0; i < NUM_FREQS_M; i++) Vboset_M[i].draw(GL_LINE_STRIP);
			for(int i = 0; i < NUM_FREQS_H; i++) Vboset_H[i].draw(GL_LINE_STRIP);
			
		ofPopMatrix();
		ofPopStyle();
	fbo[GRAPH__MIXED].end();
}

/******************************
******************************/
void ofApp::drawFbo_Or()
{
	fbo[GRAPH__OR].begin();
	shader_Or.begin();

		ofDisableAlphaBlending();
		ofFill();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		
		shader_Or.setUniformTexture( "texture1", fbo[GRAPH__M_FILLED].getTexture(), 1 );
		shader_Or.setUniformTexture( "texture2", fbo[GRAPH__H_FILLED].getTexture(), 2 );
		
		fbo[GRAPH__L_FILLED].draw(0, 0, fbo[GRAPH__L_FILLED].getWidth(), fbo[GRAPH__L_FILLED].getHeight());
	
	shader_Or.end();
	fbo[GRAPH__OR].end();
}

/******************************
******************************/
void ofApp::drawFbo_and()
{
	fbo[GRAPH__AND].begin();
	shader_And.begin();

		ofDisableAlphaBlending();
		ofFill();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		
		shader_And.setUniformTexture( "texture1", fbo[GRAPH__M_FILLED].getTexture(), 1 );
		shader_And.setUniformTexture( "texture2", fbo[GRAPH__H_FILLED].getTexture(), 2 );
		
		fbo[GRAPH__L_FILLED].draw(0, 0, fbo[GRAPH__L_FILLED].getWidth(), fbo[GRAPH__L_FILLED].getHeight());
	
	shader_And.end();
	fbo[GRAPH__AND].end();
}

/******************************
******************************/
void ofApp::drawFbo_Or_And()
{
	fbo[GRAPH__OR_AND].begin();
	shader_Or_And.begin();

		ofDisableAlphaBlending();
		ofFill();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		
		shader_Or_And.setUniformTexture( "texture1", fbo[GRAPH__AND].getTexture(), 1 );
		
		fbo[GRAPH__OR].draw(0, 0, fbo[GRAPH__OR].getWidth(), fbo[GRAPH__OR].getHeight());
	
	shader_Or_And.end();
	fbo[GRAPH__OR_AND].end();
}

/******************************
******************************/
void ofApp::drawFbo_Contour()
{
	fbo[GRAPH__CONTOUR].begin();
	shader_contour.begin();
	
		/********************
		ofFill()しないと、ofDrawRectangle()で、shaderが、枠線にしか適用されない.
		********************/
		ofDisableAlphaBlending();
		ofFill();
		
		ofClear(0, 0, 0, 0);
		ofSetColor(255, 255, 255, 255);
		
		/********************
		********************/
		shader_contour.setUniform1f("Alpha", Gui_Global->Overlay_Alpha);
		shader_contour.setUniform1f("DispGain_Sin", Gui_Global->Val_DispMax__Sin);
		shader_contour.setUniform1i("dt", (int) ((1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE) / fbo[GRAPH__CONTOUR].getHeight()));
		
		/********************
		********************/
		if(Gui_Global->b_Overlay_L)	shader_contour.setUniform1i("Low_NumFreqs", NUM_FREQS_L);
		else						shader_contour.setUniform1i("Low_NumFreqs", 0);
		
		float Low_f[NUM_FREQS_L];
		float Low_Amp[NUM_FREQS_L];
		float Low_Phase[NUM_FREQS_L];
		for(int i = 0; i < NUM_FREQS_L; i++){
			Low_f[i] = fft_thread->getFreq(FreqIds_L[i]);
			Low_Amp[i] = fft_thread->getArrayVal(FreqIds_L[i]);
			
			Low_Phase[i] = fft_thread->getPhase(FreqIds_L[i]);
			Low_Phase[i] = ofMap(Low_Phase[i], 0, 360, 0, 1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE, true);
		}
		shader_contour.setUniform1fv("Low_f", Low_f, NUM_FREQS_L);
		shader_contour.setUniform1fv("Low_Amp", Low_Amp, NUM_FREQS_L);
		shader_contour.setUniform1fv("Low_Phase", Low_Phase, NUM_FREQS_L);
		
		/********************
		********************/
		if(Gui_Global->b_Overlay_M)	shader_contour.setUniform1i("Mid_NumFreqs", NUM_FREQS_M);
		else						shader_contour.setUniform1i("Mid_NumFreqs", 0);
		
		float Mid_f[NUM_FREQS_M];
		float Mid_Amp[NUM_FREQS_M];
		float Mid_Phase[NUM_FREQS_M];
		for(int i = 0; i < NUM_FREQS_M; i++){
			Mid_f[i] = fft_thread->getFreq(FreqIds_M[i]);
			Mid_Amp[i] = fft_thread->getArrayVal(FreqIds_M[i]);
			
			Mid_Phase[i] = fft_thread->getPhase(FreqIds_M[i]);
			Mid_Phase[i] = ofMap(Mid_Phase[i], 0, 360, 0, 1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE, true);
		}
		shader_contour.setUniform1fv("Mid_f", Mid_f, NUM_FREQS_M);
		shader_contour.setUniform1fv("Mid_Amp", Mid_Amp, NUM_FREQS_M);
		shader_contour.setUniform1fv("Mid_Phase", Mid_Phase, NUM_FREQS_M);
		
		/********************
		********************/
		if(Gui_Global->b_Overlay_H)	shader_contour.setUniform1i("High_NumFreqs", NUM_FREQS_H);
		else						shader_contour.setUniform1i("High_NumFreqs", 0);
		
		float High_f[NUM_FREQS_H];
		float High_Amp[NUM_FREQS_H];
		float High_Phase[NUM_FREQS_H];
		for(int i = 0; i < NUM_FREQS_H; i++){
			High_f[i] = fft_thread->getFreq(FreqIds_H[i]);
			High_Amp[i] = fft_thread->getArrayVal(FreqIds_H[i]);
			
			High_Phase[i] = fft_thread->getPhase(FreqIds_H[i]);
			High_Phase[i] = ofMap(High_Phase[i], 0, 360, 0, 1.0e+6/AUDIO_SAMPLERATE * AUDIO_BUF_SIZE, true);
		}
		shader_contour.setUniform1fv("High_f", High_f, NUM_FREQS_H);
		shader_contour.setUniform1fv("High_Amp", High_Amp, NUM_FREQS_H);
		shader_contour.setUniform1fv("High_Phase", High_Phase, NUM_FREQS_H);
		
		/********************
		********************/
		ofDrawRectangle(0, 0, fbo[GRAPH__CONTOUR].getWidth(), fbo[GRAPH__CONTOUR].getHeight());
		
	shader_contour.end();
	fbo[GRAPH__CONTOUR].end();
}

/******************************
******************************/
void ofApp::Change_ActiveContents(int _ActiveContents_id)
{
	float now = ofGetElapsedTimef();
	
	ActiveContents_id = _ActiveContents_id;
	
	if( (StateContents == CONTENTS_WHOLE) || ((StateContents == CONTENTS_ONE) && (now - t_ContentsChange < d_ActiveContents_Transition)) ){
		t_ContentsChange = now;
		StateContents = CONTENTS_ONE;
	}
}

/******************************
******************************/
void ofApp::keyPressed(int key){
	switch(key){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			Change_ActiveContents(key - '0');
			break;
			
		case 'a':
			Change_ActiveContents(10);
			break;
			
		case 'b':
			Change_ActiveContents(11);
			break;
			
		case 'w':
			StateContents = CONTENTS_WHOLE;
			ActiveContents_id = -1;
			break;
			
		case 'd':
			b_DispGui = !b_DispGui;
			break;
			
		case 'f':
			b_DispFrameRate = !b_DispFrameRate;
			break;
			
		case ' ':
			{
				char buf[BUF_SIZE_S];
				
				sprintf(buf, "image_%d.png", png_id);
				ofSaveScreen(buf);
				// ofSaveFrame();
				printf("> %s saved\n", buf);
				
				png_id++;
			}
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

/******************************
audioIn/ audioOut
	同じthreadで動いている様子。
	また、audioInとaudioOutは、同時に呼ばれることはない(多分)。
	つまり、ofAppからaccessがない限り、変数にaccessする際にlock/unlock する必要はない。
	ofApp側からaccessする時は、threadを立てて、安全にpassする仕組みが必要
******************************/
void ofApp::audioIn(ofSoundBuffer & buffer)
{
    for (int i = 0; i < buffer.getNumFrames(); i++) {
        AudioSample.Left[i] = buffer[2*i];
		AudioSample.Right[i] = buffer[2*i+1];
    }
	
	/********************
	FFT Filtering
	1 process / block.
	********************/
	fft_thread->update__Gain(AudioSample.Left);
}  

/******************************
******************************/
void ofApp::audioOut(ofSoundBuffer & buffer)
{
	/********************
	x	:input -> output
	o	:No output.
	********************/
    for (int i = 0; i < buffer.getNumFrames(); i++) {
		buffer[2*i  ] = AudioSample.Left[i];
		buffer[2*i+1] = AudioSample.Right[i];
    }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
