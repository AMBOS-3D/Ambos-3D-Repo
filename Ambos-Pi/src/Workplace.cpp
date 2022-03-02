#include "Workplace.hpp"

using namespace cv;
using namespace std;

/**
 * @brief 	Kontruktor Default
 * @return 
 */
Workplace::Workplace(){
	rng = rng(12345);
}


/**
 * @brief 		Konstruktor
 * @param ID	Workplace ID
 * @return 
 */
Workplace::Workplace(int ID)
{
	m_ID = ID;
	m_nrBoxes = 0;
	m_boxes = NULL;
	rng = rng(12345);
}


/**
 * @brief 			Setzt die Anzahl an Boxen des Arbeitsplatzes. Gibt speicher frei, falls Array schon belegt ist.
 * @param nrBoxes
 */
void Workplace::setNrOfBoxes(int nrBoxes)
{
	m_nrBoxes = nrBoxes;
	
	// Lösche evtl. vorhandene Boxen
	if(m_boxes !=NULL)
		//delete(m_boxes);
		delete[] m_boxes;
		
	m_boxes = new Box[m_nrBoxes]; 
}


/**
 * @brief 		Hinzufügen von Boxen zum Arbeitsplatz
 * @param box	Box Objekt
 * @param nr	ID
 */
void Workplace::addBox(Box box, int nr)
{
	if(nr<m_nrBoxes)
	{
		m_boxes[nr] = box;
		
		// BoxId kann ungleich nr sein. Da Nutzer aber die Id definiert,
		// sollte diese aber als m_nr gesetzt bleiben
		//m_boxes[nr].m_nr = nr;
	}
}


	

