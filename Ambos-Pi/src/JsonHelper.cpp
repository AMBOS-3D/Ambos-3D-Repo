#include "JsonHelper.hpp"
#include <fstream>

using namespace std;
using namespace rapidjson;

/**
 * @brief 			Verändert die Attribute des Handerkennungsmodell passend zu den Settings (v2).
 * 					
 * @param model		Aktueller Detektor
 * @param settings	(neue) Settings. Enthalten sicher die notwendigen Keys! Wird zuvor im AmbosV2.cpp geprüft!
 */
void JsonHelper::updateModel(HandDetector* model, const Value* settings)
{
	model->confThreshold = (*settings)["confidence"].GetFloat();
	model->pad = (*settings)["padding"].GetInt();
	model->padFront = (*settings)["paddingFront"].GetInt();
	model->size = (*settings)["inputSize"].GetInt();
}

/**
 * @brief 			Updatet die Boxen über den Arbeitsplatz. Speichert aber die alten Zustände (leer/voll) (v2).
 * @param wp		aktueller Arbeitsplatz
 * @param boxArray	(neue) Boxen
 * @param settings	(neue) Settings , enthalten sicher die notwendigen Keys! Wird zuvor im AmbosV2.cpp geprüft!
 */
void JsonHelper::updateWorkplace(Workplace* wp, const Value* boxArray, const Value* settings)
{
	// Für den Fall, dass kein Reset (resetRequired=True in POST_CURRENT_SETTINGS) stattfindet, 
	// müssen die alten Zustände der  Boxen abgespeichert werden!
	// Reset wird wenn notwendig vom AmbosV2.cpp ausgeführt.
	vector<bool> oldEmptyFlags;
	for(int i=0; i<wp->m_nrBoxes; i++ )
	{	
		oldEmptyFlags.push_back(wp->m_boxes[i].m_boxEmpty);
	}
	
	// Setzte neue Anzahl an Boxen
	wp->setNrOfBoxes(int(boxArray->Size()));
	
	// Lade zunächst Default-Settings
	Settings s;
	
	// Setze neue LED Settings
	s.numLed = readInt(settings, "numLed");;
	s.ledStart = readInt(settings, "offsetLedStart");
	s.ledEnd = readInt(settings, "offsetLedEnd");
	
	// Erzeuge neue Boxen und weise neue Threshholds zu
	for(SizeType i=0; i< boxArray->Size();i++) {
		const Value* tempBox = &(*boxArray)[i];
		Box myBox = getBox(tempBox, &s, int(i));
		myBox.boxChangeThreshold = readInt(tempBox, "thresholdChange");
		myBox.boxEmptyThresholdColor = readFloat(tempBox, "thresholdEmptyColor");
		myBox.boxEmptyThresholdEdge = readFloat(tempBox, "thresholdEmptyEdge");
		myBox.boxEmptyViaColor = readBool(tempBox, "boxEmptyViaColor");
		
		// Setzen des alten Zuständes, wenn dieser für eine Box bekannt ist
		if(i<oldEmptyFlags.size()){
			myBox.m_boxEmpty = oldEmptyFlags[i];
		}
		
		// Wichtig, dass hier nicht myBox.m_nr sondern index i verwendet wird,
		// da m_nr nicht zwangsläufig bei 0 anfängt
		wp->addBox(myBox, i);
	}
}

/**
 * @brief 		  Erzeugen des Hand-Detektors nach den Settings
 * @param value	  Aktuelle Settings
 * @param rotate  Wichtig für frontPadding
 * @return 		  Hand Detektor
 */
HandDetector* JsonHelper::getHandDetector(Document* d, bool rotate){
	
	string pathClassesFile = readString(d, "model_classes_file");
	int size = readInt(d, "model_input_size");
	string pathModelConfiguration = readString(d, "model_cfg");
	string pathModelWeights = readString(d, "model_weights");
	float conf = readFloat(d, "model_conf_thresh");
	float nms = readFloat(d, "model_nms_thresh");
	int padding = readInt(d, "padding");
	int paddingFront = readInt(d, "padding_front");
	bool grayscale = readBool(d, "grayscale");
	return new HandDetector(size, pathClassesFile, pathModelConfiguration, 
							pathModelWeights, conf, nms, padding, paddingFront, grayscale,rotate);
}


/**
 * @brief 			Erzeugen der Box-Objekte
 * @param value		Json Objekt mit Settings aus der Order Datei 
 * @param settings	Weitere Settings
 * @param i			Box index
 * @return 			Box Objekt
 */
Box JsonHelper::getBox(const Value* tempBox, Settings* settings, int i) {
	Box box = Box();
	
	// Thresholds sind pro Box!
	// Gibt es mehr Boxen als die Arrays lang sind, wird immer das letzte Element im Array genommen => so kann man
	// auch für alle Boxen den gleichen Threshold definieren, in dem man einfach nur ein Element im Array hat
	// validIndex enthält den letzten gültigen Index dieser Arrays
	int validIndex = 0;
	
	// Wenn keine Arrays angegeben wurden, dann werden die Default Werte aus Box.hpp genommen
	
	// Wenn ein Array für thresholdChance angegeben wurde
	if(settings->thresholdChange != nullptr)
	{
		validIndex = min(i, int(settings->threshold_empty_edge->Size())-1);
		box.boxChangeThreshold = (*settings->thresholdChange)[validIndex].GetInt();
	}
	// Wenn ein Array für threshold_empty_color angegeben wurde
	if(settings->threshold_empty_color != nullptr)
	{
		validIndex = min(i, int(settings->threshold_empty_color->Size())-1);
		box.boxEmptyThresholdColor = (*settings->threshold_empty_color)[validIndex].GetFloat();
	}
	// Wenn ein Array für threshold_empty_edge angegeben wurde
	if(settings->threshold_empty_edge != nullptr)
	{
		validIndex = min(i, int(settings->threshold_empty_edge->Size())-1);
		box.boxEmptyThresholdEdge = (*settings->threshold_empty_edge)[validIndex].GetFloat();
	}
	// Wenn ein Array für emptyDetectionViaColor angegeben wurde
	if(settings->emptyDetectionViaColor != nullptr)
	{
		validIndex = min(i, int(settings->emptyDetectionViaColor->Size())-1);
		box.boxEmptyViaColor = (*settings->emptyDetectionViaColor)[validIndex].GetBool();
	}
		
	// Setze weitere Settings
	box.setParameterLED(settings->numLed, settings->ledStart, settings->ledEnd);
	// Wichtig: BoxID fängt nicht zwingend bei 0 an, also nie für Indexierung im boxArray verwenden!
	box.m_nr = readInt(tempBox, "boxID"); 
	box.name = "Box " + to_string(box.m_nr); 
	
	// In v2 gibt es keine component ID mehr
	if(tempBox->HasMember("componentID"))
	{
		box.m_componentID = readInt(tempBox, "componentID");
	}

	// Setze Koordinaten
	if (settings->rotateImage){
		box.setSize(readInt(tempBox, "y"), readInt(tempBox, "x"),
		 readInt(tempBox, "height"), readInt(tempBox, "width"));
	}
	else {
		 box.setSize(readInt(tempBox, "x"), readInt(tempBox, "y"),
		 readInt(tempBox, "width"), readInt(tempBox, "height"));
	}		
	return box;
}


/**
 * @brief 		Erzeugung einer Komponente
 * @param value	Settings
 * @param id	ID
 * @return 		Komponente
 */
Component JsonHelper::getComponent(const Value* tempBox, int id) {
	Component component = Component();
	component.m_name = readString(tempBox, "name");
	component.m_quantity = readInt(tempBox, "quantity");
	component.m_internalID = id;
	return component;
}


/**
 * @brief 			Auslesen und Erzeugung des Settings-Objekts. Wird sowohl für v1 und v2 verwendet.
 * @param value		Json Objekt mit Settings
 * @return 			Settings-Objekt
 */
Settings* JsonHelper::getSettings(Document* d, int version)
{
	
	Settings* setting = new Settings();
	
	setting->offsetRed = readInt(d, "red_offset");
	setting->offsetGreen = readInt(d, "green_offset");
	setting->offsetBlue = readInt(d, "blue_offset");
	setting->thresholdChange = readArray(d, "threshold_change");
	setting->threshold_empty_edge = readArray(d, "threshold_empty_edge");
	setting->threshold_empty_color = readArray(d, "threshold_empty_color");
	setting->numLed = readInt(d, "num_led");
	setting->ledStart = readInt(d, "offset_ledstart");
	setting->ledEnd = readInt(d, "offset_ledend");
	setting->resetPin = readInt(d, "reset_pin");
	setting->shutdownPin = readInt(d, "shutdown_pin");
	setting->visualization = readBool(d, "enable_visualization");
	setting->rotateImage = readBool(d, "rotate_image");
	setting->debug = readBool(d, "debug_text");
	setting->ledFile = readString(d, "led_file");
	setting->emptyDetectionViaColor = readArray(d, "empty_detection_via_color");
	setting->thresholdPauseModel = readInt(d, "pause_detection_threshold");
	setting->thresholdMinDetections = readInt(d, "min_number_detections");
	setting->soundsOn = readBool(d, "sounds_on");
	setting->enableHandDetection = readBool(d, "enable_hand_detection");

	if (version==2)
	{
		// workspace
		setting->workspaceId = readInt(d, "workspace_id");
		setting->workspaceName = readString(d, "workspace_name");
		
		// MQTT
		setting->mqttID = readString(d, "mqtt_id");
		setting->mqttKeepAlive = readInt(d, "mqtt_keepalive");
		setting->mqttReceiveTopic = readString(d, "mqtt_receiveTopic");
		setting->mqttSendTopic = readString(d, "mqtt_sendTopic");
		setting->mqttPort = readInt(d, "mqtt_port");
		setting->mqttHost = readString(d, "mqtt_host");
		
		// Model
		setting->modelClassesFile = readString(d, "model_classes_file");
		setting->modelInputSize = readInt(d, "model_input_size");
		setting->modelCfg = readString(d, "model_cfg");
		setting->modelWeights =  readString(d, "model_weights");
		setting->modelThreshConf = readFloat(d, "model_conf_thresh");
		setting->modelThreshNms =  readFloat(d, "model_nms_thresh");
		setting->modelPadding = readInt(d, "model_padding");
		setting->modelPaddingFront= readInt(d, "model_padding_front");
		setting->modelGrayscale = readBool(d, "model_grayscale");
	}
	return setting;
}


/**
 * @brief 					Erzeugt Workplace und Order Objekt
 * @param workplaceJson		Settings über Workplace
 * @param settings			weitere Settings
 * @return 					Order Objekt
 */
Order* JsonHelper::getOrder(Document* d, Settings* settings)
{
	if (d == NULL) {
		return nullptr;
	}
	const Value* orderJson = readObject(d, "Order");
	if (orderJson == NULL) {
		return nullptr;
	}
	const Value* boxArray = readArray(d, "BoxList");
	if (boxArray == NULL) {
		return nullptr;
	}
	const Value* componentsArray = readArray(orderJson, "components");
	if (componentsArray == NULL) {
		return nullptr;
	}
	
	// Lade Settings
	int orderID = readInt(orderJson, "orderIDNumber");
	string clientName = readString(orderJson, "orderClient");
	string name = readString(orderJson, "orderName");
	int nrComponents = int((*componentsArray).Size());
	int nrBoxes = int((*boxArray).Size());
	
	  
	// Erzeuge Workpplace
	Workplace workplace = Workplace(0);
	
	// Erzeuge Order
	Order* order = new Order(name, orderID, clientName, nrComponents, workplace, settings);
	
	// Füge Workplace Boxen hinzu. Löscht zuerst Array falls bereits vorhanden
	order->m_curWorkplace.setNrOfBoxes(nrBoxes);
	
	for(SizeType i=0; i< boxArray->Size();i++) {
		const Value* tempBox = &(*boxArray)[i];
		Box myBox = getBox(tempBox, settings, int(i));
		
		if(settings->debug)
		{
			cout << "Box " << myBox.m_nr << endl;
			cout << "\tT.Change=>" << myBox.boxChangeThreshold << endl;
			cout << "\tT.EColor=>" << myBox.boxEmptyThresholdColor << endl;
			cout << "\tT.EEdge=>" << myBox.boxEmptyThresholdEdge << endl;
			cout << "\tT.MethodColor=>" << myBox.boxEmptyViaColor << endl;
		}
		
		// Wichtig, dass hier nicht myBox.m_nr sondern index i verwendet wird,
		// da m_nr nicht zwangsläufig bei 0 anfängt
		order->m_curWorkplace.addBox(myBox, i);
	}
	
	// Füge Order Komponenten hinzu
	for(SizeType i=0; i< componentsArray->Size();i++) {
		const Value* tempComp = &(*componentsArray)[i];
		Component myComponent = getComponent(tempComp, int(i));
		order->m_compList[i] = myComponent;
	}
	return order;
}

/**
 * @brief 			Einlesen eines JSON Objekts aus Datei
 * @param filename	Datei-Filename, muss gültiges JSON Objekt enthalten
 * @return 			Pointer auf das Document
 */
Document* JsonHelper::openFile(string filename) {
	// Da mit new erzeugt, werden Objekte nach ihrem Gebrauch mit delete d gelöscht! (in util.cpp)
	Document* d = new Document();
	ifstream inputFile(filename);
	if (inputFile) {
		IStreamWrapper isw(inputFile);
		d->ParseStream(isw);	
		if (!d->IsObject()) {
			inputFile.close();
			return NULL;
		}
		inputFile.close();
	}
	else {
		return NULL;
	}
	return d;
}

/**
 * @brief 		Lese ein JSON String Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		String-Value
 */
string JsonHelper::readString(const Value* d, const char* name) {
	if (!d->HasMember(name) || !(*d)[name].IsString()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return NULL;
	}
	return (*d)[name].GetString();
}

/**
 * @brief 		Lese ein JSON Int Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		Int-Value
 */
int JsonHelper::readInt(const Value* d, const char* name){
	if (!d->HasMember(name) || !(*d)[name].IsInt()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return NULL;
	}
	return (*d)[name].GetInt();
}

/**
 * @brief 		Lese ein JSON Float Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		Float-Value
 */
float JsonHelper::readFloat(const Value* d, const char* name) {
	if (!d->HasMember(name) || !(*d)[name].IsFloat()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return NULL;
	}
	return (*d)[name].GetFloat();
}

/**
 * @brief 		Lese ein JSON Boolean Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		Boolean-Value
 */
bool JsonHelper::readBool(const Value* d, const char* name) {
	if (!d->HasMember(name) || !(*d)[name].IsBool()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return NULL;
	}
	return (*d)[name].GetBool();
}

/**
 * @brief 		Lese ein JSON Array Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		Array
 */
const Value* JsonHelper::readArray(const Value* d, const char* name){
if (!d->HasMember(name) || !(*d)[name].IsArray()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return nullptr;
	}
	
	const Value& a = (*d)[name];
	return &a;
}

/**
 * @brief 		Lese ein JSON Objekt Attribut
 * @param d		JSON Objekt
 * @param name	Key
 * @return 		JSON Objekt
 */
const Value*  JsonHelper::readObject(const Value* d, const char* name) {
	if (!d->HasMember(name) || !(*d)[name].IsObject()) {
		cout << "\033[1;31mParameter \033[0m" << name << "\033[1;31m nicht gefunden\033[0m" << std::endl;
		return nullptr;
	}
	return &(*d)[name];
}



