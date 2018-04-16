#include "Order.hpp"

using namespace std;
using namespace cv;

Order::Order(string name, int nr, string client, int nrComponents, Workplace curWorkplace)
{
	m_name = name;
	m_nr = nr;
	m_client = client;
	m_nrComponents = nrComponents;
	m_compList = new Component[m_nrComponents];
	m_curWorkplace = curWorkplace;

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
#ifdef DEBUG
	cout << "Order Reset \n" << endl;
#endif
	//Setze LEDs und BoxLEDstate zurück	
	Box* curBox;	
	for(int i=0; i<m_curWorkplace.m_nrBoxes; i++ )
	{	
		curBox = &m_curWorkplace.m_boxes[i];
		curBox->m_LEDstate = false;
	}
	setLEDs();

	return 0;
}

int Order::processOrder()
{
	
	if( OpenSPI() != 0 ) {
		cerr <<"cant open spi" << endl;
		return 0;
	}

	Line("r 2 3 4 ");		

	int orderState = 1; //Reihenfolge der Objekte, 0 = Auftrag vollständig
	int compCounter = 1; // Anzahl der entnommenen Teile, falls Reihenfolge egal 
	int lastGrab = -1; 
	
	Box* curBox;	
	Mat frame;

    //init Buttons
	pinred = 4;
	pingreen = 6;
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

#ifdef DEBUG
	cout << "Start grabbing, press key to stop" << endl;
#endif
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
#ifdef DEBUG
					cout << "Teil entnommen aus Box " << i << endl;
#endif  						
					if(false)//m_compList[curBox->m_componentID].m_packingOrderNr != 0)
					{
						//Korrekte Entnahme entprechend Reihenfolge
						if(orderState == m_compList[curBox->m_componentID].m_packingOrderNr )
						{
#ifdef DEBUG		
							cout << "Korrekte Entnahme, Packing Order Nr. " << m_compList[curBox->m_componentID].m_packingOrderNr << endl;
#endif
							curBox->m_LEDstate = true;							
							orderState++;
							orderState %= m_nrComponents;
							compCounter++;
							compCounter %= (m_nrComponents+2);
						}
						else
						{
#ifdef DEBUG
							cout << "Falsche Entnahme aus Box " << i << endl;
#endif
						}
					}
					else //Reihenfolge egal
					{
						curBox->m_LEDstate = true;
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
#ifdef DEBUG
				cout << "Auftrag vollständig \n";
#endif
				orderState = 1;
				compCounter = 1;
				//LEDs abspielen
				File(LEDFILE);
				orderReset();
			}
			//Box einzeichnen
			Point p1 = Point(curBox->m_posX, curBox->m_posY);
			Point p2 = Point(curBox->m_posX + curBox->m_sizeX, curBox->m_posY + curBox->m_sizeY);
			rectangle(frame, p1, p2, (0, 255, 0), 2, 8, 0); 
		
		}
		imshow("Camera", frame);
		int key = cv::waitKey(5);
		key = (key ==255) ? -1 : key;

		
		if(!digitalRead(pinred))
		{
#ifdef DEBUG
			std:cout << "Roter Button gedrückt - Beenden" << std::endl;
#endif
			delay(100);
			system("sudo shutdown -h now");
		}
		if(!digitalRead(pingreen))
		{
#ifdef DEBUG
			std::cout << "Grüner Button gedrückt - Reset" << std::endl;
#endif
			orderState = 1;
			compCounter = 1;
			orderReset();
			delay(200);
		}		
		else if(key>=0)
			break;

	}
	//beenden
	ResetState();
	CloseSPI() ;   

	cap.release();
	destroyAllWindows();

	return 0;

}


