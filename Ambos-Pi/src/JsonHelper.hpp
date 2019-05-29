#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include "json.h"
#include "Order.hpp"
#include "Settings.hpp"

class JsonHelper{
//Funktionen
public:
	//Öffnet das übergebene json-File und speichert die Daten in file_contents
	json_value* openJsonFile(char* filename);
	//schließt das aktuell geöffnete json-File und räumt auf
	void closeJsonFile();
	//Lädt den Auftrag im json-File und gibt ihn mit den geladenen Parametern zurück
	Order* getOrder(json_value* value, Settings* settings);
	//Lädt die Settings im json-File
	Settings* getSettings(json_value* value);
private:
	//Hilfsfunktion um eine Box einzulesen
	Box getBox(json_value* value, Settings* settings);
	//Hilfsfunktion um eine Komponente einzulesen
	Component getComponent(json_value* value, int id);
	//Funktionen zum casten der Datentypen
	int getInteger(char* name, json_value* value);
	json_value* getObject(char* name, json_value* value);
	std::string getString(char* name, json_value* value);
	bool getBoolean(char* name, json_value* value);

//Attribute
private:
	//speichert temporär den aktuellen Wert
	json_value* value;
	//Pointer zum Dateiinhalt
	char* file_contents;
};
