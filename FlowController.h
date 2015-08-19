
#ifndef FlowController_h
#define FlowController_h

#include "Arduino.h"

class FlowController
{
public:
	FlowController(int actualFlowInputPin);
	~FlowController(void);
	void setSetpoint(float setpoint);
	void setKFactor(float kFactor);
	void setInputAdcFullScale(int adcMax);
	void setSetpointAdcFullScale(int adcMax);
	void setFlowFullScale(float fullScale);
	float getActualFlow();
	float getSetpoint();
	int getSetpointAdcValue();
	float getKFactor();
	float getInputAdcToFlowFactor();
	float getSetpointFlowToAdcFactor();
	

private:
	int _actualFlowInputPin;
	float _setpoint;
	float _kFactor;
	float _maximumFlow;
	int _inputAdcFullScale;
	int _setpointAdcFullScale;
	
	float _inputAdcToFlowFactor;
	float _setpointFlowToAdcFactor;

	void updateInputAdcToFlowFactor();
	void updateSetpointFlowToAdcFactor();
};

#endif