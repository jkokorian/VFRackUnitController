
#ifndef AD5263_h
#define AD5263_h

class AD5263
{
public:
	AD5263(int chipSelectPin, int shutDownPin);
	~AD5263(void);
	void writeChannelValue(int channel, int value);
	void deactivate();
	void activate();
	void initialize();
private:
	int _csPin;
	int _shutDownPin;
};

#endif