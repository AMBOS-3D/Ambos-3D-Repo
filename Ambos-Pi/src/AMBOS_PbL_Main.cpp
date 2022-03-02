#include "AMBOS_PbL_Main.hpp"
#include <iostream>
#include <stdlib.h>
#include "Settings.hpp"
#include "HandDetector.hpp"
#include "AmbosV2.hpp"
#include "util.h"
#include <thread>

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{	
	// erstes Argument entscheided, welche Version gestartet werden soll (Integer)
	int version = atoi(argv[1]);
	printf("\033[1;32m### Starte Ambos Version %i ##################\033[0m\n", version);

	// Setze feste Pfade der Konfigurationsdateien
	char* fSettings = (char*) "/home/ambos/workspace/Ambos-Pi/data/settings/settings.json";
	char* fModel = (char*)"/home/ambos/workspace/Ambos-Pi/data/settings/model_settings.json";
	char* fSettingsV2 = (char*)"/home/ambos/workspace/Ambos-Pi/data/v2/settings/settings.json";
	
	if(version==1)
	{
		/**
		 * In Version 1 wird die Applikation nach dem Boot über einen internen Webserver (./bin/server/webserver.py)  gestartet. 
		 * Die Applikation arbeitet den Auftrag <fOrder> ab. Erkannte Griffe und leere Boxen führen direkt
		 * zur Änderung der LEDs und Ton. Die Prozessteuerung übernimmt die Applikation selbst.
		 */
		
		// Für Ambos V1 benötigt man eine Order Datei (String)
		char* fOrder  = argv[2];

		// Lade allgemeine Settings
		Settings* curSettings = loadSettings(fSettings, 1);
		
		// Übergebe Hand-Detektor Settings und lade das Objekt falls enableHandDetection==True
		HandDetector* model = nullptr;
		if(curSettings->enableHandDetection)
		{
			model = loadMlModel(fModel,curSettings->rotateImage);
		}
		else{
			cout << "\033[1;31mHanderkennung ausgeschaltet\033[0m" << endl;
		}

		// Spiele Start-sound, falls aktiviert
		if(curSettings->soundsOn)
		{	
			playSound("sudo aplay /home/ambos/workspace/Ambos-Pi/data/sounds/start.wav  >>/dev/null 2>>/dev/null");
		}
		
		// Übergebe die Order Datei und lade das Objekt
		Order* curOrder = loadOrder(fOrder, curSettings);
		
		// Lösche die eingelesen JSON Dateien (erzeugt mit new), da diese nicht mehr gebraucht werden
		freeJsonDocs();
		
		// Wenn die Order fehlerhaft ist, kann die Applikation nicht gestartet werden
		if(curOrder != nullptr)
		{
			// Starte den Auftragsablauf
			curOrder->processOrder(model, curSettings->soundsOn);
		}
	}
	else
	{
		/**
		 * In Version 2 wird keine Order Datei eingelesen. Die Logik sitzt in der ControlApp. Dort wird eine Auftrag angelegt.
		 * Zu Beginn werden alle Objekte (AmbosV2Handler, FrameHandler, MQTT, Workplace, HandDetector) mit Default-Werten
		 * initialisiert. Über die POST_CURRENT_SETTINGS MQTT Nachricht werden diese Attribute dann aktualisiert. Ambos meldet
		 * erkannte Griffe und leere Boxen über MQTT Nachrichten an die ControlApp
		 */
		 
		// Lade Settings (v2). Alle Settings sind dort enthalten
		Settings* curSettings = loadSettings(fSettingsV2, 2);
		
		// Spiele Start-sound wenn aktiviert
		if(curSettings->soundsOn)
		{	
			playSound("sudo aplay /home/ambos/workspace/Ambos-Pi/data/sounds/start.wav >>/dev/null 2>>/dev/null");
		}
		
		// Lade Klassen für die Ablaufsteuerung und MQTT Verbindung
		AmbosV2Handler ambosv2 =  AmbosV2Handler(curSettings);
		MqttConnector mqtt =  MqttConnector(curSettings);
		ambosv2.setMqtt(&mqtt);
		
		// Gebe Json Objekte der Default-Werte wieder frei
		freeJsonDocs();
		
		/**
		 * Applikation wird beendet, wenn keine MQTT client erstellt werden konnte.
		 *  Dies bezieht sich nicht darauf, ob schon eine Verbindung zum Broker erstellt werden konnte.
		 */
		if(mqtt.mosqClient != nullptr)
		{
			/**
			 * Sollte keine Verbindung zum Broker aufgebaut werden, so befindet sich der Prozess in
			 * einer while-Schleife und versucht sich ständig neu zu verbinden
			 */
			mqtt.startClient();
			
			/**
			 * Erst wenn eine Verbindung zum Broker besteht, startet die Erkennung für alle Boxen. 
			 * Ohne den Erhalt einer POST_CURRENT_SETTINGS Nachricht, sind aber keine Boxen eingespeichert.
			 */
			ambosv2.start();
		}
	}
	
	std::getchar();
	return 0;
}
