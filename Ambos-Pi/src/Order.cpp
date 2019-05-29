#include "Order.hpp"

using namespace std;
using namespace cv;

Order::Order(string name, int nr, string client, int nrComponents, Workplace curWorkplace, Settings* newSettings)
{
	m_name = name;
	m_nr = nr;
	m_client = client;
	m_nrComponents = nrComponents;
	m_compList = new Component[m_nrComponents];
	m_curWorkplace = curWorkplace;
	settings = *newSettings;

}

void Order::setLEDs()
{
	Box* curBox;
	string redLEDs = "r ";	
	string greenLEDs = "g ";	
	string blueLEDs = "b ";
	for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
	{	
		curBox = &m_curWorkplace.m_boxes[i];
		if(!curBox->m_LEDstate)
		{
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
			stringstream ss;
			ss << curBox->LED_Position << " ";
			string str = ss.str();
			greenLEDs +=  str;
		}
		if(curBox->m_boxEmpty)
		{
			stringstream ss;
			ss << curBox->LED_Position - 1 << " " << curBox->LED_Position + 1 << " ";
			string str = ss.str();
			blueLEDs += str;
		}

	}	

	string s_line = redLEDs + greenLEDs + blueLEDs;
	const char* line = s_line.c_str();
	Line(line);

}

int Order::orderReset()
{
	if(settings.debug) {
		cout << "Order Reset \n" << endl;
	}
	//Setze LEDs und BoxLEDstate zurück	
	Box* curBox;	
	for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
	{	
		curBox = &m_curWorkplace.m_boxes[i];
		curBox->m_LEDstate = false;
		curBox->m_componentCounter = 0;
	}
	setLEDs();

	return 0;
}

int Order::processOrder()
{
	if( OpenSPI(settings.offset_red, settings.offset_green, settings.offset_blue) != 0 ) {
		cerr <<"cant open spi" << endl;
		return 0;
	}

	Line("g 1 ");		

	int orderState = 1; //Reihenfolge der Objekte, 0 = Auftrag vollständig
	int compCounter = 1; // Anzahl der entnommenen Teile, falls Reihenfolge egal 
	int lastGrab = -1; 
	
	Box* curBox;	
	Mat frame;
    //init Buttons
	pinred = settings.shutdown_pin;
	pingreen = settings.reset_pin;
	if (wiringPiSetup () < 0)
	{
		cerr << "setup failed\n" << endl;
		exit (1) ;
	}
        
    pinMode(pinred, INPUT);
    pullUpDnControl(pinred, PUD_UP);
    pinMode(pingreen, INPUT);
	pullUpDnControl(pingreen, PUD_UP);

	VideoCapture cap(0);
	if(!cap.isOpened())
	{
		cerr << "ERROR: Unable to open camera" << endl;
		return 0;
	}

	if(settings.debug) {
		cout << "Start grabbing, press key to stop" << endl;
	}
	while(1)
	{
		cap >> frame;
		if (frame.empty())
		{
			cerr << "ERROR: Unable to grab from camera" << endl;
			break;
		}
		//Konvertieren in Grauwertbild
		Mat frameGray;
		cvtColor( frame, frameGray, COLOR_BGR2GRAY );
		//Boxen	einzeln durchgehen	
		for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
		{		
			curBox = &m_curWorkplace.m_boxes[i];		
			curBox->setBoxImage(frameGray);
			if(curBox->contourInBox())
			{
				//letzte Entnahme erfolgte aus Box i
				lastGrab = i;
			}
			else
			{
				if(curBox->changeInBox() && (lastGrab==i)) //Änderung in Box, in der die letzte Entnahme erfolgte
				{	
					//Korrekte Entnahme
					if(settings.debug) {
						cout << "Teil entnommen aus Box " << i << endl;
					}			
					if(false)//m_compList[curBox->m_componentID].m_packingOrderNr != 0)
					{
						//Korrekte Entnahme entprechend Reihenfolge
						if(orderState == m_compList[curBox->m_componentID].m_packingOrderNr )
						{
							if(settings.debug) {
								cout << "Korrekte Entnahme, Packing Order Nr. " << m_compList[curBox->m_componentID].m_packingOrderNr << endl;
							}
							curBox->m_LEDstate = true;							
							orderState++;
							orderState %= m_nrComponents;
							compCounter++;
							compCounter %= (m_nrComponents+2);
						}
						else
						{
							if(settings.debug) {
								cout << "Falsche Entnahme aus Box " << i << endl;
							}
						}
					}
					else //Reihenfolge egal
					{
						curBox->m_componentCounter++;						
						if(curBox->m_componentCounter >= m_compList[curBox->m_componentID].m_quantity)
						{	
							curBox->m_LEDstate = true;
						}	
						compCounter++;
						compCounter %= (m_nrComponents+1);
					}
				}
			}
			setLEDs();
			bool orderFinished = true;
	        for(int k=0; k<m_curWorkplace.m_nrBoxes; k++ )
            {
		      	if(!m_curWorkplace.m_boxes[k].m_LEDstate)
				{
					orderFinished = false;
				}
			}
			if(orderFinished)
			{
				if(settings.debug) {
					cout << "Auftrag vollständig \n";
				}
				orderState = 1;
				compCounter = 1;
				//LEDs abspielen
				File(settings.ledFile.c_str());
				orderReset();
			}
			//Box einzeichnen
			if(settings.visualization){
				Point p1 = Point(curBox->m_posX, curBox->m_posY);
				Point p2 = Point(curBox->m_posX + curBox->m_sizeY, curBox->m_posY + curBox->m_sizeX);
				rectangle(frame, p1, p2, cv::Scalar(0, 140, 255), 1, 8, 0); 
			}
		}
		
		if(!digitalRead(pinred))
		{
			if(settings.debug) {
				std:cout << "Roter Button gedrückt - Beenden" << std::endl;
			}
			delay(100);
			system("shutdown -h now");
		}
		if(!digitalRead(pingreen))
		{
			if(settings.debug) {
				std::cout << "Grüner Button gedrückt - Reset" << std::endl;
			}
			orderState = 1;
			compCounter = 1;
			orderReset();
			delay(100);
		}		

		if(settings.visualization){
			namedWindow("AMBOS-3D", WINDOW_NORMAL);
			resizeWindow("AMBOS-3D", 1280, 960);
			imshow("AMBOS-3D", frame);
			int key = cv::waitKey(1);
			key = (key ==255) ? -1 : key;	
			// Reset, wenn 'r' gedrückt wird
		
			if ((key == 'r') || (key =='R'))
			{	
				orderState = 1;
				compCounter = 1;			
				orderReset();
			}	 	
		}
	}
	//beenden
	ResetState();
	CloseSPI() ;   

	cap.release();
	destroyAllWindows();

	return 0;

}


