#pragma once
#include <iostream>

#include <core/core.hpp>
#include <highgui/highgui.hpp>

#include "Box.hpp"

//Parameter definieren
#define MAX_THRESH 255;
#define THRES 140;

class Workplace
{
//Funktionen
	public:
	//Konstruktoren
	Workplace();
	Workplace(int ID);
	//Setzt die Anzahl der Boxen
	void setNrOfBoxes(int nrBoxes); 	
	void addBox(Box box, int nr);
	private:
//Nicht vollständig implementiertes Feature: 
	//automatische Boxsuche im Bild
	int findBoxesInImg();	
	//Hilfsfunktion für findBoxesInImg()
	void thresh_callback(int, void* );
	int grabCurImage(); 
//Attribute
	public:
	int m_ID;
	cv::Mat m_curImage;
	int m_nrBoxes;
	Box* m_boxes; 
	private:
	int thresh;
	cv::RNG rng;
	int max_thresh;
	cv::Mat blurImg;

};
