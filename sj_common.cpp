/************************************************************
************************************************************/
#include "sj_common.h"

/************************************************************
************************************************************/
/********************
********************/
int GPIO_0 = 0;
int GPIO_1 = 0;

/********************
********************/
GUI_GLOBAL* Gui_Global = NULL;

FILE* fp_Log = NULL;
FILE* fp_Log_main = NULL;
FILE* fp_Log_Audio = NULL;
FILE* fp_Log_fft = NULL;


/************************************************************
func
************************************************************/
/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha_dt, double dt)
{
	double Alpha;
	if((Alpha_dt <= 0) || (Alpha_dt < dt))	Alpha = 1;
	else									Alpha = 1/Alpha_dt * dt;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double LPF(double LastVal, double CurrentVal, double Alpha)
{
	if(Alpha < 0)		Alpha = 0;
	else if(1 < Alpha)	Alpha = 1;
	
	return CurrentVal * Alpha + LastVal * (1 - Alpha);
}

/******************************
******************************/
double sj_max(double a, double b)
{
	if(a < b)	return b;
	else		return a;
}


/************************************************************
class
************************************************************/

/******************************
******************************/
void GUI_GLOBAL::setup(string GuiName, string FileName, float x, float y)
{
	/********************
	********************/
	gui.setup(GuiName.c_str(), FileName.c_str(), x, y);
	
	/********************
	********************/
	Group_Graph.setup("Graph");
		Group_Graph.add(LPFAlpha_dt__FFTGain.setup("LPF_dt:Gain", 0.15, 0, 2.0));
		Group_Graph.add(LPFAlpha_dt__FFTPhase.setup("LPF_dt:Phase", 0.045, 0, 0.3));
		Group_Graph.add(Val_DispMax__FFTGain.setup("ValDisp FFT", 0.05, 0, 0.1));
		Group_Graph.add(Val_DispMax__Sin.setup("ValDisp Sin", 0.03, 0, 0.1));
	gui.add(&Group_Graph);
		
	Group_LMH.setup("LMH Range");
		Group_LMH.add(Id_Low.setup("Low", 2, 1, 255));
		Group_LMH.add(Id_Middle.setup("Mid", 10, 1, 255));
		Group_LMH.add(Id_High.setup("High", 20, 1, 255));
	gui.add(&Group_LMH);
		
	Group_Overlay.setup("Overlay");
		Group_Overlay.add(b_Do_Overlay.setup("Do overlay", false));
		Group_Overlay.add(Overlay_Alpha.setup("Alpha", 0.7, 0, 1.0));
		Group_Overlay.add(b_Overlay_L.setup("L", true));
		Group_Overlay.add(b_Overlay_M.setup("M", true));
		Group_Overlay.add(b_Overlay_H.setup("H", true));
	gui.add(&Group_Overlay);

	/********************
	********************/
	gui.minimizeAll();
}

