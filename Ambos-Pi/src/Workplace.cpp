#include "Workplace.hpp"

using namespace cv;
using namespace std;


Workplace::Workplace(){
	rng = rng(12345);
	thresh = THRES;
	max_thresh = MAX_THRESH;
}

Workplace::Workplace(int ID)
{
	m_ID = ID;
	m_nrBoxes = 0;
	m_boxes = NULL;
	rng = rng(12345);
	thresh = THRES;
	max_thresh = MAX_THRESH;
}

void Workplace::setNrOfBoxes(int nrBoxes)
{
	m_nrBoxes = nrBoxes;
	//lösche evtl. vorhandene Boxen
	if(m_boxes !=NULL)
		delete(m_boxes);
	m_boxes = new Box[m_nrBoxes]; 
}



void Workplace::addBox(Box box, int nr)
{
	if(nr<m_nrBoxes)
	{
		m_boxes[nr] = box;
		m_boxes[nr].m_nr = nr;
		//Initialisiere Bild
		m_boxes[nr].setBoxImage(m_curImage);
	}
}

int Workplace::grabCurImage()
{
	VideoCapture cap(0);
	if(!cap.isOpened())
	{
		cerr << "ERROR: Unable to open camera" << endl;
		return 0;
	}

	Mat wsFrame;

	cap >> wsFrame;
	if (wsFrame.empty())
	{
		cerr << "ERROR: Unable to grab from camera" << endl;
	}	

	//Konvertieren in Grauwertbild
	Mat wsFrameGray;
	cvtColor( wsFrame, wsFrameGray, COLOR_BGR2GRAY );	
	m_curImage = wsFrameGray;
	cap.release();

}


void Workplace::thresh_callback(int, void* )
{

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat threshImg;
	threshold(blurImg, threshImg, thresh, 255, THRESH_BINARY);
	
	findContours(threshImg, contours, hierarchy, RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
 	
	vector<vector<Point> > contours_poly( contours.size() );
  	vector<Rect> boundRect( contours.size() );	

	int j = 0;
	for( size_t i = 0; i < contours.size(); i++ )
  	{
    	approxPolyDP( Mat(contours[i]), contours_poly[i], 1, true );
		if (contourArea(contours_poly[i]) > 10000)
    	{
			boundRect[j] = boundingRect( Mat(contours_poly[i]) );
			j++;
		}
  	}
  
	Mat drawing = Mat::zeros( threshImg.size(), CV_8UC3 );
  
	for( size_t i = 0; i< j; i++ )
	{
	    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
    	drawContours( drawing, contours_poly, (int)i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    	rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
    
  	}
  	
}

int Workplace::findBoxesInImg()
{
	blur(m_curImage, blurImg, Size(3,3));

	if(m_curImage.empty())
		grabCurImage();

	const char* source_window = "Source";
  	namedWindow( source_window, WINDOW_AUTOSIZE );
  	imshow( source_window, m_curImage );
  	//createTrackbar( " Threshold:", "Source", &thresh, max_thresh, Workplace::thresh_callback );
  	//thresh_callback( 0, 0 );
  	
	waitKey(0);
	

}	


	

