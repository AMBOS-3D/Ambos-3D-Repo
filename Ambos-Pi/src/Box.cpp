#include "Box.hpp"

using namespace cv;
using namespace std;


Box::Box(void)
{
	m_posX = 0;
	m_posY = 0;
	m_sizeX = 1;
	m_sizeY = 1;
	m_componentCounter = 0;
	m_boxEmpty = false;
	m_LEDstate = false;
}

Box::Box(int x, int y, int cx, int cy)
{
	m_posX = x;
	m_posY = y;
	m_sizeX = cx;
	m_sizeY = cy;
	m_componentCounter = 0;
 	m_boxEmpty = false;
	m_LEDstate = false;

	m_boxImg = Mat(m_sizeY, m_sizeX, CV_8UC1);
	m_boxBg = Mat(m_sizeY, m_sizeX, CV_8UC1);

	calculateLEDPosition();	

	m_bgsubtractor=createBackgroundSubtractorMOG2(50, 16, false);
}

void Box::setParameterThresholds(int change, int empty){
	boxChangeThreshold = change;
	boxEmptyThreshold = empty;
}

void Box::setParameterLED(int numLED, int startLED, int endLED){
	num_LED = numLED;
	start_LED = startLED;
	end_LED = endLED;
}

void Box::setSize(int x, int y, int cx, int cy)
{
	m_posX = x;
	m_posY = y;
	m_sizeX = cx;
	m_sizeY = cy;

	m_boxImg = Mat(m_sizeY, m_sizeX, CV_8UC1);
	m_boxBg = Mat(m_sizeY, m_sizeX, CV_8UC1);

	calculateLEDPosition();	

	m_bgsubtractor=createBackgroundSubtractorMOG2(50, 16, false);

}

void Box::setBoxImage(Mat src)
{	
	if((src.cols>=(m_posX + m_sizeX))&&(src.rows>=(m_posY + m_sizeY)))	
	{
		Rect ROI(m_posX, m_posY, m_sizeX, m_sizeY);
		m_boxImg = Mat(src, ROI);
	}
	else
		return;
}

bool Box::changeInBox()
{
	//Background bisher nicht initialisiert	
	if (m_boxBg.empty()){
		m_boxBg = m_boxImg;
	}
	Mat frameDelta; 
	Mat threshBg;
	//prüfe, ob sich der Background geändert hat zu letztem mal
	absdiff(m_boxBg, m_boxImg, frameDelta);	
	//Hintergrundbild speichern
	m_boxBg = m_boxImg;
 	
	threshold(frameDelta, threshBg, 25, 255, THRESH_BINARY);
	int NbrWhite = countNonZero(threshBg);
	//Teste Histogrammfunktion	
	m_boxEmpty = isBoxEmpty(); 
	if(NbrWhite>(boxChangeThreshold)){ 
		return  true;
	}
	else{
		return false; 
	}

}

bool Box::contourInBox()
{
	//Gauss Blur bestimmen und als Maske anwenden
	Mat frameGrBlur, bgMask;
	GaussianBlur(m_boxImg, frameGrBlur, Size(5,5),0);
	m_bgsubtractor->apply(frameGrBlur, bgMask);
	//Maske vergroessern
	Mat thresh;
	dilate(bgMask, thresh, Mat(), Point(-1,-1), 1);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(thresh, contours, hierarchy, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);	
	if(contours.size()> 0 ){ //KEIN Parameter, Teileerkennung schlägt fehl wenn kleine Konturen ignoriert werden
		return true;
	}
	else{
		return false;
	}
}

void Box::calculateLEDPosition()
{
	int strip_size = end_LED - start_LED; //pixel count of LED strip
	float center = m_posX + (m_sizeX)/2 - start_LED; //center point of box between start and end position of LED strip
	float LED_size = strip_size/num_LED + 1;
	LED_Position = (int)(center/LED_size) + 1;
}

bool Box::isBoxEmpty()
{
	MatND hist;
   	int sbins = 256;
    int histSize[] = {sbins};
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 256 };
    const float* ranges[] = { sranges };
 	int channels[] = {0};
	calcHist(&m_boxImg, 1, channels, Mat(), hist, 1,  histSize, ranges,  true, // the histogram is uniform
             false );
    double maxVal=0;
    minMaxLoc(hist, 0, &maxVal, 0, 0);

	if (maxVal > boxEmptyThreshold){ 
		return true;
	}
	else{
		return false;
	}
}

