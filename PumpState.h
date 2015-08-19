#pragma once
#include "VFOutputState.h"
#include "VFSystemConstants.h"
class PumpState :
	public VFOutputState
{
public:

	PumpState()
	{
		_vacuumPumpActive = false;
		_bubblerInletValveOpen = false;
		_bubblerOutletValveOpen = false;
		_pureArgonValveOpen = false;
		_chamberInletValveOpen = false;
		_chamberOutletValveOpen = false;
		_pumpValveOpen = false;
		_ventValveOpen = false;
		_pureArgonFlowSetpoint = 0;
		_bubblerFlowSetpoint = 0;
	}

	~PumpState()
	{
	}

	bool PumpState::isAllowed(VFInputState inputState) {
		if (inputState.getBubblerFlowVolume() > CHAMBER_VOLUME) {
			return false;
		}
		else {
			
		}
	}
};

