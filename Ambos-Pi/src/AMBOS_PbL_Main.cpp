#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <core/core.hpp>
#include <highgui/highgui.hpp>
#include "JsonHelper.hpp"
#include "Settings.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
	JsonHelper json;
    json_value* value_order;
	json_value* value_settings;
	char* filename  = argv[1];
	char* settings_file = (char*)"/home/ambos/AMBOS-PbL/data/settings/settings.json";
	Order* curOrder = nullptr;
	Settings* curSettings = nullptr;
	//lade Settings
	value_settings = json.openJsonFile(settings_file);
	if (value_settings == NULL) {
	        cerr << "Unable to parse data\n" << endl;
	        json.closeJsonFile();
			//use default values
			curSettings = new Settings();
	}
	else {
		//Parse Json-Objekt Settings
		if (value_settings->type == json_object)
		{
			cout << "Loading Settings..." << endl;
			curSettings = json.getSettings(value_settings);
		}
		json.closeJsonFile();
	}
	//lade Order und setzte die im json hinterlegten Parameter	
	while(curOrder == nullptr)
	{
		value_order = json.openJsonFile(filename);
		if (value_order == NULL) {
		        cerr << "Unable to parse data\n" << endl;
		        json.closeJsonFile();
				continue;
		}
		//Parse Json-Objekt Order
		if (value_order->type == json_object)
		{
			cout << "New Order\n" << endl;
		    curOrder = json.getOrder(value_order, curSettings);
		}
		json.closeJsonFile();
		//Konfiguration Workplace 
		if (curOrder == nullptr)
		{
			cerr << "nullptr exception\n" << endl;
			sleep(100);
		}
	}
	//starte den Auftragsablauf
	int e = curOrder->processOrder();
	waitKey(0);
	return 0;
}
