#pragma once
#include <string>

class Settings{
//Attribute
public:
	int offset_red = 0;
	int offset_green = 2;
	int offset_blue = 1;
	int threshold_change = 10;
	int threshold_empty = 800;
	int num_led = 25;
	int led_start = 85;
	int led_end = 555;
	int reset_pin = 6;
	int shutdown_pin = 4;
	bool visualization = true;
	bool rotate_image = false;
	bool debug = false;
	std::string ledFile;
};

