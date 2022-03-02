#pragma once
#include <iostream>
#include <core/core.hpp>
#include <imgproc/imgproc.hpp>
#include <video/background_segm.hpp>


/**
 * @class Box
 * @brief Speichert die eingezeichneten Boxen. Jedes Box Objekt hält die konfigurierten Thresholds, welche für die
 * 		  Erkennung von Konturen, Entnahmen und leere Boxen notwendig sind. Diese Prozessschritte sind jeweils als
 * 		  Klassenfunktion hier implementiert. Für die leere Box Erkennung können 2 Methoden gewählt werden. Jede Box
 * 		  besitzt einen BackgroundSubtractorMOG2 um statische Elemente herauszulöschen
 */
class Box
{
	// Funktionen
	public:
		// Konstruktoren
		Box(void);
		Box(int x, int y, int cx, int cy);
		
		// Größe der Box verändern
		void setSize(int x, int y, int cx, int cy);
		
		// Setzt das aktuelle Image der Box gemäß den in den Attributen hinterlegten Werten
		// Schneidet das volle Bild auf den Boxbereich zu
		void setBoxImage(cv::Mat src);
		
		// Berechnet die Änderung zwischen dem "älteren" Hintergrundbild und dem aktuellen Boxausschnitt
		bool changeInBox();
		
		// Prüft, ob sich aktuell eine Kontur im Bildausschnitt der Box befindet
		bool contourInBox();
		
		// Setzt die Parameter für die LEDs auf gewünschte Werte. Daraus wird die LED_Position berechnet
		void setParameterLED(int numLED, int startLED, int endLED);
		
		// Erkennung, ob Box leer ist  (mehr zu den Methoden in box.cpp)
		bool isBoxEmpty();
			
		// Setzte das aktuelle Boxbild als Hintergrund (wichtig für changeInBox)
		void updateBoxBgImg();
	private:
		// Berechnet die aktuelle Position der LEDs anhand fest hinterlegter Parameter
		void calculateLEDPosition();
		
	//Attribute
	public:
		// Position om Bild in Pixel
		int m_posX;
		int m_posY;
		
		// Größe in Pixel
		int m_sizeX;
		int m_sizeY;
		
		// Weitere Parameter aus Order
		int m_nr;
		int m_componentID;
		bool m_boxEmpty;
		bool m_LEDstate; // false = rot, true = grün	oder gelb
		
		// Wie viele Teile wurden aus Box entnommen
		int m_componentCounter; 
		
		// LED Nummer auf Leiste
		int LED_Position;
		
		// Name der Box
		std::string name = "";
		
		// Parameter: Welche Änderung führt zu Erkennung einer Entnahme
		int boxChangeThreshold = 10;
		
		// Falls Erkennung der Boxfülle über Farbhistogram geht,
		// steuert der Parameter ab wann eine Box als leer erkannt wird.
		// Float-Wert definiert wieviel Prozent der Pixel die gleiche Farbe haben müssen (für eine leere Box)
		float boxEmptyThresholdColor = 0.01;
		
		// Falls Erkennung der Boxfülle über Kanten geht,
		// steuert der Parameter ab wann eine Box als leer erkannt wird.
		// Float-Wert definiert wieviel Prozent der Pixel max. zu Kanten gehören dürfen (für eine leere Box)
		float boxEmptyThresholdEdge = 0.1;
				
		// Wenn false, dann werden leere Boxen über den Kanten-basierten Ansatz entdeckt.
		bool boxEmptyViaColor = false;
		
	private:
		// Aktueller Bildauschschnitt der Box und zugehöriger Hintergrund
		cv::Mat m_boxImg; 
		cv::Mat m_boxBg;
		
		// Trennt bewegte Objekte vom statischen Hintergrund
		// Nur auf den bewegten Objekte wird contourInBox angewendet
		cv::Ptr<cv::BackgroundSubtractorMOG2> m_bgsubtractor;
		
		// Parameter für die LED Position
		int num_LED;
		int start_LED;
		int end_LED;
};
