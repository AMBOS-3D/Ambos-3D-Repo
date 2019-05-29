#pragma once
#include <wiringPi.h>
#include <string>
#include "Component.hpp"
#include "Workplace.hpp"
#include <unistd.h>
#include "ledstripPbL.hpp"
#include <iostream>
#include <stdlib.h>
#include "Settings.hpp"


class Order
{
	public:

	std::string m_name;
	int m_nr;
	std::string m_client;

	//FillerSlideType filler;
	Component* m_compList;
	int m_nrComponents;	
	Workplace m_curWorkplace;

	//Taster
	int pinred;
	int pingreen;

	//Constructor	
	Order(std::string name, int nr, std::string client, int nrComponents, Workplace curWorkplace, Settings* newSettings);

	//Ablaufsteuerung
	int processOrder();

	//Ansteuerung des LED Strips
	//liest den aktuellen Zustand aus dem Order-Objekt und setzt passend die LEDs
	void setLEDs();

	//manuelles Reset per Knopfdruck ODER nach vollständigem Prozess 
	int orderReset();

	//Settings
	Settings settings;

};
