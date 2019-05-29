#include "JsonHelper.hpp"

using namespace std;

json_value* JsonHelper::openJsonFile(char* filename)
{
	FILE *fp;
	struct stat filestatus;
	int file_size;
	json_char* json;

    if ( stat(filename, &filestatus) != 0) {
            fprintf(stderr, "File %s not found\n", filename);
            return NULL;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
            fprintf(stderr, "Memory error: unable to allocate %d bytes\n", file_size);
            return NULL;
    }

    fp = fopen(filename, "rt");
    if (fp == NULL) {
            fprintf(stderr, "Unable to open %s\n", filename);
            fclose(fp);
            free(file_contents);
            return NULL;
    }
    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
            fprintf(stderr, "Unable t read content of %s\n", filename);
            fclose(fp);
            free(file_contents);
            return NULL;
    }
    fclose(fp);
    json = (json_char*)file_contents;
    value = json_parse(json,file_size);
	return value;

}

void JsonHelper::closeJsonFile()
{
	json_value_free(value);
    free(file_contents);
}

// 

Box JsonHelper::getBox(json_value* value, Settings* settings) {
	Box box = Box();
	box.setParameterThresholds(settings->threshold_change, settings->threshold_empty);
	box.setParameterLED(settings->num_led, settings->led_start, settings->led_end);
	box.m_componentID = getInteger((char *)"componentID", value);
	box.m_nr = getInteger((char *)"boxID", value);
	if (settings->rotate_image){
		box.setSize(getInteger((char *)"y1", value), getInteger((char *)"x1", value),
		 getInteger((char *)"width", value), getInteger((char *)"height", value));
	}
	else {
		box.setSize(getInteger((char *)"x1", value), getInteger((char *)"y1", value),
		 getInteger((char *)"height", value), getInteger((char *)"width", value));
	}		
	return box;
}

Component JsonHelper::getComponent(json_value* value, int id) {
	Component component = Component();
	component.m_name = getString((char *)"name", value);
	component.m_packingOrderNr = getInteger((char *)"packingOrder", value);
	component.m_posOnSlide = getInteger((char *)"fillerSlidePlace", value);
	component.m_quantity = getInteger((char *)"quantity", value);
	component.m_onSlide = getBoolean((char *)"withinFillerSlide", value);
	component.m_inBox = getBoolean((char *)"withinBox", value);
	component.m_internalID = id;
	return component;
}

Settings* JsonHelper::getSettings(json_value* value)
{
	Settings* setting = new Settings();
	setting->offset_red = getInteger((char *)"red_offset", value);
	setting->offset_green = getInteger((char *)"green_offset", value);
	setting->offset_blue = getInteger((char *)"blue_offset", value);
	setting->threshold_change = getInteger((char *)"threshold_change", value);
	setting->threshold_empty = getInteger((char *)"threshold_empty", value);
	setting->num_led = getInteger((char *)"num_led", value);
	setting->led_start = getInteger((char *)"offset_ledstart", value);
	setting->led_end = getInteger((char *)"offset_ledend", value);
	setting->reset_pin = getInteger((char *)"reset_pin", value);
	setting->shutdown_pin = getInteger((char *)"shutdown_pin", value);
	setting->visualization = getBoolean((char *)"enable_visualization", value);
	setting->rotate_image = getBoolean((char *)"rotate_image", value);
	setting->debug = getBoolean((char *)"debug_text", value);
	setting->ledFile = getString((char *)"led_file",value);
	return setting;
}

Order* JsonHelper::getOrder(json_value* workplaceJson, Settings* settings)
{

	json_value* tempComponent;
	json_value* tempBox;
	
	if (workplaceJson == NULL) {
		return nullptr;
	}
	json_value* orderJson = getObject((char *)"Order", workplaceJson);
	if (orderJson == NULL) {
		return nullptr;
	}
	json_value* fillerSlide = getObject((char *)"fillerSlide", orderJson);
	if (fillerSlide == NULL) {
		return nullptr;
	}
	json_value* boxArray = getObject((char *)"BoxList", workplaceJson);
	if (boxArray == NULL) {
		return nullptr;
	}
	json_value* componentsArray = getObject((char *)"components", orderJson);
	if (componentsArray == NULL) {
		return nullptr;
	}

	int orderID = getInteger((char *)"orderIDNumber", orderJson);
	string clientName = getString((char *)"orderClient", orderJson);
	string name = getString((char *)"orderName", orderJson);
	int nrComponents = getInteger((char *)"numComponents", orderJson);

	Workplace workplace = Workplace(0);
	Order* order = new Order(name, orderID, clientName, nrComponents, workplace, settings);

	order->m_curWorkplace.setNrOfBoxes(getInteger((char *)"numBoxes", workplaceJson));

	for(int i = 0; i < boxArray->u.array.length; i++) {
		tempBox = boxArray->u.array.values[i];
		Box myBox = getBox(tempBox, settings);
		order->m_curWorkplace.addBox(myBox, myBox.m_nr);
		cout << "Box " << myBox.m_nr << endl;
	}
	for(int i = 0; i < componentsArray->u.array.length; i++) {
		tempComponent = componentsArray->u.array.values[i];
		Component myComponent = getComponent(tempComponent, i);
		order->m_compList[i] = myComponent;
		cout << "Component " << myComponent.m_name << endl;
	}
	return order;
}

//Funktionen zum casten der Datentypen

json_value* JsonHelper::getObject(char* name, json_value* value) {
	if(value->type == json_object) {
		for(int i = 0; i < value->u.object.length; i++) {
			if (!strcmp(value->u.object.values[i].name, name)) {
				return value->u.object.values[i].value;
			}
		}
	}
	return NULL;
}

string JsonHelper::getString(char* name, json_value* value) {
	if(value->type == json_object) {
		for(int i = 0; i < value->u.object.length; i++) {
			if (!strcmp(value->u.object.values[i].name, name)) {
				return value->u.object.values[i].value->u.string.ptr;
			}
		}
	}
	return nullptr;
}

int JsonHelper::getInteger(char* name, json_value* value) {
	if(value->type == json_object) {
		for(int i = 0; i < value->u.object.length; i++) {
			if (!strcmp(value->u.object.values[i].name, name)) {
				return value->u.object.values[i].value->u.integer;
			}
		}
	}
	return -1;
}

bool JsonHelper::getBoolean(char* name, json_value* value) {
	if(value->type == json_object) {
		for(int i = 0; i < value->u.object.length; i++) {
			if (!strcmp(value->u.object.values[i].name, name)) {
				return value->u.object.values[i].value->u.boolean;
			}
		}
	}
	return false;
}


