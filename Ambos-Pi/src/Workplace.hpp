#pragma once
#include <iostream>
#include "Box.hpp"

/**
 * @class Workplace
 * @brief Arbeitsplatz Objekt. Enthält Array mit Box Objekten
 */
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

//Attribute
	public:
		int m_ID = 0;
		int m_nrBoxes = 0;
		Box* m_boxes; 
		std::string name;
	private:	
		cv::RNG rng;
};
