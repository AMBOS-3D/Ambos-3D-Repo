#include "Component.hpp"

using namespace std;

Component::Component(void){
}

Component::Component(string name, int internalID, int quantity, bool onSlide, bool inBox, int packingOrderNr)
{
	m_name = name;
	m_quantity = quantity;
	m_onSlide = onSlide;
	m_inBox  = inBox;
	m_packingOrderNr = packingOrderNr;
	m_internalID = internalID;
}
