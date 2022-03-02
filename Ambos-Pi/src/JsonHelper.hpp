#pragma once
#include <stdio.h>
#include <string>
#include "Order.hpp"
#include "Settings.hpp"
#include "HandDetector.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

using namespace rapidjson;

/**
 * @class JsonHelper
 * @brief Klasse zum einlesen von JSON Objekten/Attributen. Werden v.a. von util.cpp aufgerufen
 */
class JsonHelper
{
	//Funktionen
	public:
		// Lädt den Auftrag im json-File und gibt ihn mit den geladenen Parametern zurück
		Order* getOrder(Document* d, Settings* settings);
		
		// Lädt die Settings 
		Settings* getSettings(Document* d, int version);
		
		// Lädt den Hand-Detector
		HandDetector* getHandDetector(Document* d, bool rotate);
		
		// Aktualisiert die Attribute des Hand-Detektors (v2)
		void updateModel(HandDetector* model, const Value* settings);
		
		// Aktualisiert die Attribute des Arbeitsplatzes und die darin enthaltenen Boxen (v2)
		void updateWorkplace(Workplace* wp, const Value* boxArray, const Value* settings);
		
		// JSON-Parsing Funktionen
		rapidjson::Document* openFile(std::string filename);
		std::string readString(const Value* d, const char* name);
		int readInt(const Value* d, const char* name);
		float readFloat(const Value* d, const char* name);
		bool readBool(const Value* d, const char* name);
		const Value* readArray(const Value* d, const char* name);
		const Value* readObject(const Value* d, const char* name);
		
	private:
		// Hilfsfunktion um eine Box einzulesen
		Box getBox(const Value* tempBox, Settings* settings, int i);
		
		// Hilfsfunktion um eine Komponente einzulesen
		Component getComponent(const Value* tempBox, int id);
};
