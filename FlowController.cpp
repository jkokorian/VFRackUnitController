#include "FlowController.h"


FlowController::FlowController(int actualFlowInputPin)
{
	pinMode(actualFlowInputPin, INPUT);
	_actualFlowInputPin = actualFlowInputPin;
}


FlowController::~FlowController(void)
{
}


void FlowController::setInputAdcFullScale(int adcMax)
{
	_inputAdcFullScale = adcMax;
	updateInputAdcToFlowFactor();
}

void FlowController::setSetpointAdcFullScale(int adcMax) 
{
	_setpointAdcFullScale = adcMax;
	updateSetpointFlowToAdcFactor();
}

void FlowController::setFlowFullScale(float fullScale)
{
	_maximumFlow = fullScale;
	updateInputAdcToFlowFactor();
	updateSetpointFlowToAdcFactor();
}

void FlowController::setKFactor(float kFactor)
{
	_kFactor = kFactor;
	updateInputAdcToFlowFactor();
	updateSetpointFlowToAdcFactor();
}

void FlowController::updateInputAdcToFlowFactor()
{
	_inputAdcToFlowFactor = ( _maximumFlow * _kFactor ) / _inputAdcFullScale;
}

void FlowController::updateSetpointFlowToAdcFactor() 
{
	_setpointFlowToAdcFactor = _setpointAdcFullScale / ( _maximumFlow * _kFactor);
}


float FlowController::getActualFlow() 
{
	int adcValue = analogRead(_actualFlowInputPin);
	return adcValue * _inputAdcToFlowFactor;
}

void FlowController::setSetpoint(float setpoint)
{
	_setpoint = (setpoint > _maximumFlow ? _maximumFlow : setpoint);
	
}

float FlowController::getSetpoint()
{
	return _setpoint;
}

int FlowController::getSetpointAdcValue()
{
	return _setpoint * _setpointFlowToAdcFactor;
}

float FlowController::getKFactor()
{
	return _kFactor;
}

float FlowController::getInputAdcToFlowFactor() {
	return _inputAdcToFlowFactor;
}

float FlowController::getSetpointFlowToAdcFactor() {
	return _setpointFlowToAdcFactor;
}
