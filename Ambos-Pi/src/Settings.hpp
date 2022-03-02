#pragma once
#include <string>
#include <iostream>
#include <rapidjson/document.h>

using namespace rapidjson;
using namespace std;

/**
 * @class Settings
 * @brief Settings Objekt, wird sowohl von v1 und v2 verwendet, weshalb manche Attribute in v1 dann nicht gesetzt sind
 */
class Settings{
//Attribute
public:
	// LED Offsets
	int offsetRed = 0;
	int offsetGreen = 2;
	int offsetBlue = 1;
	// LED Details
	int numLed = 25;
	int ledStart = 85;
	int ledEnd = 555;
	string ledFile;
	
	// Pin Details
	int resetPin = 6;
	int shutdownPin = 4;
	
	// Allgemeine Settings
	bool visualization = true;
	bool rotateImage = false;
	bool debug = true;
	bool enableHandDetection = true;
	bool soundsOn = true;
	
	// Erkennung von leeren Boxen
	// wenn false, dann über Edge detection
	const Value* emptyDetectionViaColor = nullptr;
	
	// Schwellwert für emptyBoxDetectionMethod=0: Je größer desto mehr freie Fläche wird benötigt
	const Value* threshold_empty_color = nullptr;
	// Schwellwert für emptyBoxDetectionMethod=1: Je kleiner desto mehr freie Fläche wird benötigt
	const Value* threshold_empty_edge = nullptr;
	
	// Weitere Schwellwerte
	// Gibt an wie stark sich Box-Bildinhalt ändern muss, damit ein Griff abgeleitet wird
	const Value* thresholdChange = nullptr;
	// Nachdem eine Kontour erkannt wurde im Bild wird nach dieser Anzahl Frames, in welchen
	// eine Hand erkannt wurde, die Handerkennung pausiert, bis keine Kontour mehr erkannt ist.
	int thresholdPauseModel = 10;	
	// Nur wenn in den voherigen Schritten in mind. sovielen Frames eine Handerkannt wurde,
	// wird auch die Change-.Erkennung durchgeführt
	int thresholdMinDetections = 1;
	
	// MQTT Settings, nur für V2
	string mqttID = "PI_Client";
	string mqttReceiveTopic = "CONTROL_MSG";
	string mqttSendTopic = "SENSOR_MSG";
	string mqttHost = "localhost";
	int mqttPort = 1883;
	int mqttKeepAlive = 60;
	
	// ML hand detection, nur für V2
	string modelClassesFile = "/home/ambos/workspace/Ambos-Pi/data/detector_config/yolo/hand.names";
	int modelInputSize= 160;
	string modelCfg = "/home/ambos/workspace/Ambos-Pi/data/detector_config/yolo/tiny-yolo_new.cfg";
	string modelWeights =  "/home/ambos/workspace/Ambos-Pi/data/detector_config/yolo/tiny-yolo_best_new.weights";
	float modelThreshConf = 0.3;
	float modelThreshNms =  0.1;
	int modelPadding = 20;
	int modelPaddingFront=0;
	bool modelGrayscale=false;
	
	int workspaceId;
	string workspaceName="";
};

