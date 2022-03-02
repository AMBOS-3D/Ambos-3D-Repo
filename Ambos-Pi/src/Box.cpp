#include "Box.hpp"
#include <imgproc/imgproc.hpp>
#include <string>
#include "opencv2/opencv.hpp"
#include <opencv2/dnn.hpp>
#include <opencv2/dnn/dnn.hpp>
using namespace cv;
using namespace std;

/**
 * @brief Default Konstruktor
 * @return 
 */
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

/**
 * @brief Konstruktor setzt die Boxkoordinaten
 * @param x		
 * @param y
 * @param cx	Breite
 * @param cy	Höhe
 * @return 
 */
Box::Box(int x, int y, int cx, int cy)
{
	m_posX = x;
	m_posY = y;
	m_sizeX = cx;
	m_sizeY = cy;
	
	// Default-Werte
	m_componentCounter = 0; 
 	m_boxEmpty = false;
	m_LEDstate = false;
	
	// Platzhalter für den Bildausschnitt der Box
	m_boxImg = Mat(m_sizeY, m_sizeX, CV_8UC1);

	calculateLEDPosition();	
	
	//	Initalisierung, trennt später die bewegten Objekt vom Hintergrund
	m_bgsubtractor=createBackgroundSubtractorMOG2(50, 16, false);
}

/**
 * @brief Jeder Box wird eine LED zugeordnet mit einer bestimmten Position im Stripe
 * @param numLED
 * @param startLED
 * @param endLED
 */
void Box::setParameterLED(int numLED, int startLED, int endLED){
	num_LED = numLED;
	start_LED = startLED;
	end_LED = endLED;
}

/**
 * @brief Boxgröße zuweisen/ändern
 * @param x
 * @param y
 * @param cx
 * @param cy
 */
void Box::setSize(int x, int y, int cx, int cy)
{
	m_posX = x;
	m_posY = y;
	m_sizeX = cx;
	m_sizeY = cy;

	m_boxImg = Mat(m_sizeY, m_sizeX, CV_8UC1);
	calculateLEDPosition();	
	m_bgsubtractor=createBackgroundSubtractorMOG2(50, 16, false);
}

/**
 * @brief Setzt das aktuelle Bild der Box auf m_boxImg
 * @param src	Aktuelle Frame in voller Auflösung
 */
void Box::setBoxImage(Mat src)
{	
	if((src.cols>=(m_posX + m_sizeX))&&(src.rows>=(m_posY + m_sizeY)))	
	{
		// Zuschneiden des Frames auf den Boxbereich
		Rect ROI(m_posX, m_posY, m_sizeX, m_sizeY);
		m_boxImg = Mat(src, ROI);
		
	}
	else
		return;
}

/**
 * @brief 	Kopiere das aktuelle Boxbild in die Variable m_boxBg (Hintergrundbild)
 */
void Box::updateBoxBgImg()
{
	m_boxBg = m_boxImg.clone();
}

/**
 * @brief 	Zwischen den aktuellen Boxbild und dem gespeicherten
 * 			Boxhintergrund wird die Differenz berechnet
 * @return 	True oder False jenachdem ob eine Änderung erkannt wurde
 */
bool Box::changeInBox()
{
	Mat frameDelta; 
	Mat threshBg;
	
	// Prüfe, ob sich der Background geändert hat zu letztem mal
	absdiff(m_boxBg, m_boxImg, frameDelta);	
	
	// Betrachte nur aussagekräftige Änderungen
	threshold(frameDelta, threshBg, 25, 255, THRESH_BINARY);
	
	// Schwellwert erreicht?
	int NbrWhite = countNonZero(threshBg);
	if(NbrWhite>(boxChangeThreshold)){ 
		return  true;
	}
	else
	{
		return false; 
	}
}

/**
 * @brief 	Suche im aktuellen Boxbild nach bewegten Konturen
 * @return 	True oder False jenachdem ob eine Kontur erkannt wurde
 */
bool Box::contourInBox()
{	
	// Gauss Blur bestimmen und als Maske anwenden
	Mat frameGrBlur, bgMask;
	GaussianBlur(m_boxImg, frameGrBlur, Size(5,5),0);
	
	// Hintergrund abziehen 
	m_bgsubtractor->apply(frameGrBlur, bgMask);

	// Maske vergroessern
	Mat thresh;
	dilate(bgMask, thresh, Mat(), Point(-1,-1), 1);
	
	// Konturen finden
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(thresh, contours, hierarchy, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);	
	string title = "";

	// Jede Kontor wird gemeldet
	if(contours.size() > 0 ){
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief Berechnet die Position der Box-LED im LED-Strip
 */
void Box::calculateLEDPosition()
{
	// Berechne länge des LED Strip
	int strip_size = end_LED - start_LED; 
	
	// Zentrum Box zwischen Start und Endposition des LED Strip
	float center = m_posX + (m_sizeX)/2 - start_LED; 
	float LED_size = strip_size/num_LED + 1;
	LED_Position = (int)(center/LED_size) + 1;
	// Sicherstellen, dass die mittlere LED nicht am Anfang oder Ende ist.
	LED_Position = max(LED_Position, 2);
	LED_Position = min(LED_Position, num_LED-1);
}

/**
 * @brief 	Erkennung einer leeren Box, aktuell mit 2 verschiedenen Methoden:
 * 			A. Sättigung-Histogramm => beruht auf der Annahme, dass in leeren Boxen mehr Pixel die gleiche Farbe (Sättigung)
 * 			   haben als volle. Eine leere Box wird also dann erkannt, wenn der größte Wert im Sättigungs-Histogramm über einem
 *             Schwellwert liegt
 *          B. Kantenerkennung => beruht auf der Annahme, dass in leeren Boxen weniger Kanten zu erkennen sind als in vollen.
 *             Eine leere Box wird also dann erkannt, wenn die Anzahl der Pixel, welche einer Kante zugeordnet sind, unter einem
 *             Schwellwert liegt.
 * @return 	True oder False jenachdem ob die Box leer oder voll ist
 */
bool Box::isBoxEmpty()
{	
	// Erkennung basieren aufm Farbhistrogram
	if (boxEmptyViaColor)
	{
		MatND hist;
		int sbins = 256;
		int histSize[] = {sbins};
		
		// Sättigung zwischen 0 und 255
		float sranges[] = { 0, 256 };
		const float* ranges[] = { sranges };
		int channels[] = {0};
		
		// Histogram ist Uniform
		calcHist(&m_boxImg, 1, channels, Mat(), hist, 1,  histSize, ranges,  true, 
				 false );
		
		// Gesamtpixel
		int n_pixels = m_boxImg.size().width*m_boxImg.size().height;
		
		// Hole Maximalwert
		double maxVal=0;
		minMaxLoc(hist, 0, &maxVal, 0, 0);
		
		// Bilde Verhältnis
		float ratio = maxVal / n_pixels;
		
		// Über Schwellwert?
		if (ratio > boxEmptyThresholdColor){ 
			return true;
		}
		else{
			return false;
		}
	}
	else
	{
		Mat gray, blurred, imgCanny, dilation;
		
		// Blur 
		GaussianBlur(m_boxImg, blurred, Size(3,3), 0);
		
		// Kantenerkennung über Canny
		Canny(blurred, imgCanny, 150, 250);
		
		int dilationSize = 5;
		
		// Vergrößern und lücken schließen
		Mat element = getStructuringElement(2, Size(2*dilationSize+1, 2*dilationSize+1), Point(dilationSize, dilationSize));
		dilate(imgCanny, dilation, element);
		
		// Zähle Pixel ungleich null (=Kanten)
		int e_count = countNonZero(dilation);
		float ratio = (float)e_count/(dilation.size().width*dilation.size().height);
		
		// kleiner als Schwellwert?
		if (ratio < boxEmptyThresholdEdge){
			return true;
		}else{
			return false;
		}
	}
}

