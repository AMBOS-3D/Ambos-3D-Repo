#pragma once
#include <iostream>
#include <core/core.hpp>
#include <imgproc/imgproc.hpp>
#include <video/background_segm.hpp>




class Box
{
//Funktionen
public:
	//Konstruktoren
	Box(void);
	Box(int x, int y, int cx, int cy);
	//Größe der Box verändern
	void setSize(int x, int y, int cx, int cy);
	//setzt das aktuelle Image der Box gemäß den in den Attributen hinterlegten Werten
	//Eingangsparamter: das aktuelle, vollständige Kamerabild
	void setBoxImage(cv::Mat src);
	//Prüft ob eine Änderung in der Box eingetroffen ist zwischen dem als letzten
	//Background gespeicherten Bild und dem aktuell gesetzten und setzt hinterher
	//den Background neu mit dem aktuellen Bild
	bool changeInBox();
	//prüft, ob sich aktuell eine Kontur im Bildausschnitt der Box befindet
	bool contourInBox();
	//setzt die Parameter für die Erkennung auf gewünschte Werte
	void setParameterThresholds(int change, int empty);
	//setzt die Parameter für die LEDs auf gewünschte Werte
	void setParameterLED(int numLED, int startLED, int endLED);
private:
	//berechnet die aktuelle Position der LEDs anhand fest hinterlegter Parameter
	void calculateLEDPosition();
	//prüft ob die Box leer ist und setzt dementsprechend m_boxEmpty auf true
	bool isBoxEmpty();
//Attribute
public:
	//Position om Bild in Pixel
	int m_posX;
	int m_posY;
	//Größe in Pixel
	int m_sizeX;
	int m_sizeY;
	//weitere Parameter aus Order
	int m_nr;
	int m_componentID;
	bool m_boxEmpty;
	bool m_LEDstate; //false = rot, true = grün	oder gelb
	//Wie viele Teile wurden aus Box entnommen
	int m_componentCounter; 
	//LED Nummer auf Leiste
	int LED_Position;

private:
	//aktueller Bildauschschnitt der Box und zugehöriger Hintergrund
	cv::Mat m_boxImg; 
	cv::Mat m_boxBg;
	cv::Ptr<cv::BackgroundSubtractorMOG2> m_bgsubtractor;
	//Parameter: Wann ist eine Änderung eingetreten?
	int boxChangeThreshold = 10;
	//Parameter: gibt an, ab wann die Box als leer 
	//bezeichnet wird. Je größer, desto mehr leere Fläche wird benötigt
	int boxEmptyThreshold = 800;
	//Parameter für die LED Position
	int num_LED;
	int start_LED;
	int end_LED;

};
