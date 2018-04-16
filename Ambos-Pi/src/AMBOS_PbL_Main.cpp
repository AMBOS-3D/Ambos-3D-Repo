#include "AMBOS_PbL_Main.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
	JsonHelper json;
    json_value* value;
	char* filename  = argv[1];
	Order* curOrder = nullptr;
	//lade Order und setzte die im json hinterlegten Parameter	
	while(curOrder == nullptr)
	{
		value = json.openJsonFile(filename);
		if (value == NULL) {
		        cerr << "Unable to parse data\n" << endl;
		        json.closeJsonFile();
				continue;
		}
		//Parse Json-Objekt Order
		if (value->type == json_object)
		{
			cout << "New Order\n" << endl;
		    curOrder = json.getOrder(value);
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
