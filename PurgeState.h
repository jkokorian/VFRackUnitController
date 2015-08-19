#pragma once
#include "VFOutputState.h"
class PurgeState :
	public VFOutputState
{
public:

	PurgeState()
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

	~PurgeState()
	{
	}

	void setPureArgonFlowSetpoint(int value) {
		_pureArgonFlowSetpoint = value;
	}
};

