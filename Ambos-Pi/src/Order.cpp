#include "Order.hpp"
#include "util.h"
#include "FrameHandler.h"
#include "ledstripPbL.hpp"
#include <wiringPi.h>

using namespace std;
using namespace cv;
using namespace std::chrono;

/**
 * @brief 				Konstruktor
 * @param name			Name der Order
 * @param nr			Nr
 * @param client		Client
 * @param nrComponents	Anzahl an Komponenten
 * @param curWorkplace	Arbeitsplatz Objekt
 * @param newSettings	Settings Objekt
 * @return 
 */
Order::Order(string name, int nr, string client, int nrComponents, Workplace curWorkplace, Settings* newSettings)
{
	m_name = name;
	m_nr = nr;
	m_client = client;
	m_nrComponents = nrComponents;
	m_compList = new Component[m_nrComponents]; // Komponenten Liste
	m_curWorkplace = curWorkplace;
	settings = *newSettings;
}


/**
 * @brief 	Setzt die Farben der LEDS. Der Status der LEDs wird einmal verändert wenn alle Komponenten 
 * 			aus einer Box entnommen worden sind (m_LEDstate) oder die Box leer ist (curBox->m_boxEmpty).
 * 			Für jede Box muss eine LED-Position berechnet werden
 */
void Order::setLEDs()
{
	Box* curBox;
	string redLEDs = "r ";	
	string greenLEDs = "g ";	
	string blueLEDs = "b ";
	
	// Iteriere über alle Boxen
	for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
	{	
		curBox = &m_curWorkplace.m_boxes[i];
		
		if(!curBox->m_LEDstate)
		{
			// Es müssen noch Teile entnommen werden
			stringstream ss;
			ss << curBox->LED_Position << " ";
			string str = ss.str();
			redLEDs +=  str;
		
			if(curBox->m_componentCounter > 0 ){			
				greenLEDs +=  str;
			}
		}
		else
		{
			// Keine Teile mehr notwendig aus dieser Box
			stringstream ss;
			ss << curBox->LED_Position << " ";
			string str = ss.str();
			greenLEDs +=  str;
		}
		
		// Setze LED auf Blau wenn Box leer ist
		if(curBox->m_boxEmpty)
		{
			stringstream ss;
			ss << curBox->LED_Position - 1 << " " << curBox->LED_Position + 1 << " ";
			string str = ss.str();
			blueLEDs += str;
		}

	}	
	
	// Schreiben auf den LED Stripe
	string s_line = redLEDs + greenLEDs + blueLEDs;
	const char* line = s_line.c_str();
	Line(line);
}


/**
 * @brief 		Resettet den Auftrag. Setzt LED Status zurück und die Komponenten Zähler
 * @return 
 */
int Order::orderReset()
{
	if(settings.debug) {
		cout << "\n\n### Starte Auftrag neu ###" << endl;
	}
	
	// Setze LEDs und BoxLEDstate zurück	
	Box* curBox;	
	for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
	{	
		curBox = &m_curWorkplace.m_boxes[i];
		curBox->m_LEDstate = false;
		curBox->m_componentCounter = 0;
		if(settings.debug) 
		{
			printf(" Box %i : 0/%i \n", i, m_compList[curBox->m_componentID].m_quantity);
		}
	}
	printf("##################\n\n");
	
	setLEDs();

	return 0;
}


/**
 * @brief 		Setzen der Taster auf GPIO Pins
 */
void Order::initGPIO()
{
	// Festlegen der Pins aus den Settings
	pinred = settings.shutdownPin;
	pingreen = settings.resetPin;
	
	if (wiringPiSetup () < 0)
	{
		cerr << "setup failed\n" << endl;
		exit (1) ;
	}
	
	// Setzen der Pins als Input
    pinMode(pinred, INPUT);
    pullUpDnControl(pinred, PUD_UP);
    pinMode(pingreen, INPUT);
	pullUpDnControl(pingreen, PUD_UP);
}


/**
 * @brief 	LED Pattern wenn eine Packung fertig ist
 */
void Order::orderFinishLED()
{
	File(settings.ledFile.c_str());
}


/**
 * @brief 				Ablaufsteuerung über diese Funktion. Funktion baut den Fraumhandler ruft diesen
 * 						kontinuierlich auf
 * @param model			Hand Detektor
 * @param soundsOn		Töne an?
 * @return 
 */
int Order::processOrder(HandDetector* model, bool soundsOn)
{	
	if( OpenSPI(settings.offsetRed, settings.offsetGreen, settings.offsetBlue) != 0 ) {
		cerr <<"cant open spi" << endl;
		return 0;
	}
	
	// LED Start Sequenz
	File("/home/ambos/workspace/Ambos-Pi/data/led/start.led");
	
	
	// Init GPIO
	initGPIO();
	
	// init Kamera
	VideoCapture cap = getCap();
	
	if(settings.debug) 
	{
		printf("\033[1;32mStarte mit dem Packprozess!\033[0m\n");
	}
	
	// Baue den Framehandler
	FrameHandler fh = FrameHandler(&m_curWorkplace, nullptr, cap, model, 
										this, settings.debug, settings.visualization, 
										settings.rotateImage, settings.thresholdPauseModel, 
										settings.thresholdMinDetections);
	fh.soundsOn = soundsOn;
	
	while(1)
	{	
		// Verarbeite den aktuellen Frame
		fh.processFrame();
		
		// Schaue ob ein Pin gedrückt ist
		if(!digitalRead(pinred))
		{
			if(settings.debug) 
			{
				cout << "Roter Button gedrückt - Beenden" << endl;
			}
			delay(100);
			Line(""); // alle leds aus
			system("shutdown -h now");
		}			
		if(!digitalRead(pingreen))
		{
			if(settings.debug) 
			{
				std::cout << "Grüner Button gedrückt - Reset" << std::endl;
			}
			orderReset(); // setze Auftrag zurück
			fh.reset(false);	// Setze Iterationsabhängige Variablen zurück
			delay(100);
		}		
	}
	
	// Beenden
	ResetState();
	CloseSPI() ;   
	cap.release();
	destroyAllWindows();

	return 0;

}


