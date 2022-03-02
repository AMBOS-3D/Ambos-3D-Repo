#pragma once
#include <string>

/**
 * @class Component
 * @brief Enthält die in der Order-Datei festgelegten Komponenten 
 */
class Component
{
	// Funktionen
	public:	
		//Konstruktoren
		Component();
		Component(std::string name, int internalID, int quantity);
	
	// Attribute
	public:
		std::string m_name; // Hinterlegter Name
		int m_internalID; // Laufende Nummer im Auftrag
		int m_quantity; // Anzahl der Teile
};
