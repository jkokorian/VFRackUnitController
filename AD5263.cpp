#include "Arduino.h"
#include "SPI.h"
#include "AD5263.h"

AD5263::AD5263(int chipSelectPin, int shutDownPin)
{
	_csPin = chipSelectPin;
	_shutDownPin = shutDownPin;
	pinMode(chipSelectPin, OUTPUT);
	pinMode(shutDownPin, OUTPUT);
}


AD5263::~AD5263(void)
{

}

void AD5263::initialize()
{
	SPI.begin();
	digitalWrite(_shutDownPin,LOW); //deactivated by default
	digitalWrite(_csPin,HIGH); //deselect chip by default
}

void AD5263::writeChannelValue(int channel, int value)
{
	// take the SS pin low to select the chip:
	digitalWrite(_csPin,LOW);
	//  send in the address and value via SPI:
	SPI.transfer(channel);
	SPI.transfer(value);
	// take the SS pin high to de-select the chip:
	digitalWrite(_csPin,HIGH);	
}

void AD5263::deactivate()
{
	digitalWrite(_shutDownPin,LOW);
}

void AD5263::activate()
{
	digitalWrite(_shutDownPin,HIGH);
}




