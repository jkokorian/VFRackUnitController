#pragma once
#include "State.h"
#include "VFInputState.h"
class VFOutputState :
	public State
{
public:
	VFOutputState();
	~VFOutputState();
	virtual bool getValuesHaveChanged() const;
	virtual bool isAllowed(VFInputState* inputState) const;
	void apply() const;

	bool getBubblerInletValveOpen() const;
	bool getBubblerOutputValveOpen() const;
	bool getPureArgonValveOpen() const;
	bool getChamberInletValveOpen() const;
	bool getChamberOutletValveOpen() const;
	bool getVentValveOpen() const;
	bool getPumpValveOpen() const;
	bool getVacuumPumpActive() const;
	int getPureArgonFlowSetpoint() const;
	int getBubblerFlowSetpoint() const;

protected:
	bool _bubblerInletValveOpen;
	bool _bubblerOutletValveOpen;
	bool _pureArgonValveOpen;
	bool _chamberInletValveOpen;
	bool _chamberOutletValveOpen;
	bool _ventValveOpen;
	bool _pumpValveOpen;
	bool _vacuumPumpActive;
	int _pureArgonFlowSetpoint;
	int _bubblerFlowSetpoint;

};

