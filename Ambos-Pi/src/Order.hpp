#pragma once
#include <string>
#include "Component.hpp"
#include "Workplace.hpp"
#include "Settings.hpp"
#include "HandDetector.hpp"

/**
 * @class Order
 * @brief Zentrales Objekt für V1. Main-loop wird in processOrder gestartet. Für die Verarbeitung wird ein FrameHandler Objekt verwendet
 */
class Order
{		
	// Funktionen
	public:
		
		// Konstruktor	
		Order(std::string name, int nr, std::string client, int nrComponents, Workplace curWorkplace, Settings* newSettings);

		// Ablaufsteuerung
		int processOrder(HandDetector* model, bool soundsOn);

		// Ansteuerung des LED Strips
		// liest den aktuellen Zustand aus dem Order-Objekt und setzt passend die LEDs
		void setLEDs();

		// Manuelles Reset per Knopfdruck ODER nach vollständigem Prozess 
		int orderReset();
		
		// Initialisiere die Buttons
		void initGPIO();
		
		// LED-Show wenn Auftrag/Packung fertig
		void orderFinishLED();
		
		// Settings
		Settings settings;
		
	//Attribute
	public:
		// Order Attribute
		std::string m_name;
		int m_nr;
		std::string m_client;
		
		
		// Taster
		int pinred; // für das Ausschalten
		int pingreen; // für Reset
		
		// Komponenten und Workplace Objekt
		Component* m_compList;
		int m_nrComponents;	
		Workplace m_curWorkplace;

};	
