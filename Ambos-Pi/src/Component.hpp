#pragma once
#include <string>

class Component
{
//Funktionen
	public:	
	//Konstruktoren
	Component();
	Component(std::string name, int internalID, int quantity, bool onSlide, bool inBox, int packingOrderNr);
//Attribute
	public:
	std::string m_name; //hinterlegter Name
	int m_internalID; //Laufende Nummer im Auftrag
	int m_quantity; //Anzahl der Teile
	bool m_onSlide; //liegt auf dem Füllschieber?
	bool m_inBox; //liegt in einer Box?
	int m_posOnSlide; //Position auf Füllschieber als Nummer
	int m_packingOrderNr; //legt die Reihenfolge fest
};
