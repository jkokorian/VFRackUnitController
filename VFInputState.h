#include "Arduino.h"
#include "HardwareConstants.h"
#pragma once
class VFInputState
{
public:
	
	VFInputState()
	{
		_actualBubblerFlow = 0;
		_actualPureArgonFlow = 0;
		_actualChamberPressure = 1000;
		_timestampLastUpdated = 0;
		resetIntegratedValues();
	}

	~VFInputState()
	{
	}

	void resetIntegratedValues() {
		_pureArgonFlowVolume = 0;
		_bubblerFlowVolume = 0;
	}

	void update() {

		unsigned long t = millis();
		//TODO: update inputs

		double dt = (double)(t - _timestampLastUpdated) / 1000.0;
		_timestampLastUpdated = t;
		
		if (dt != 0) {
			_pureArgonFlowVolume = _actualPureArgonFlow * dt;
			_bubblerFlowVolume = _actualBubblerFlow * dt;
		}
	}

private:
	int _actualPureArgonFlow;
	int _actualBubblerFlow;
	int _actualChamberPressure;
	
	int _timestampLastUpdated;
	double _pureArgonFlowVolume;
	double _bubblerFlowVolume;

};

