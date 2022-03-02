#include "Component.hpp"

using namespace std;

/**
 * @brief Default Konstrunktor
 * @return 
 */
Component::Component(void){
}

/**
 * @brief Setze entsprechende Attribute
 * @param name			Name der Komponente
 * @param internalID	ID
 * @param quantity		Häufigkeit
 * @return 
 */
Component::Component(string name, int internalID, int quantity)
{
	m_name = name;
	m_quantity = quantity;
	m_internalID = internalID;
}
