#include "VFOutputState.h"
#include "Arduino.h"

int VFOutputState::getBubblerFlowSetpoint() const
{
	return _bubblerFlowSetpoint;
}

bool VFOutputState::getBubblerInletValveOpen() const {
	return _bubblerInletValveOpen;
}

bool VFOutputState::getBubblerOutputValveOpen() const {
	return _bubblerOutletValveOpen;
}

bool VFOutputState::getChamberInletValveOpen() const {
	return _chamberInletValveOpen;
}

bool VFOutputState::getChamberOutletValveOpen() const {
	return _chamberOutletValveOpen;
}

bool VFOutputState::getPumpValveOpen() const {
	return _pumpValveOpen;
}

int VFOutputState::getPureArgonFlowSetpoint() const {
	return _pureArgonFlowSetpoint;
}

bool VFOutputState::getPureArgonValveOpen() const {
	return _pureArgonValveOpen;
}

bool VFOutputState::getVacuumPumpActive() const {
	return _vacuumPumpActive;
}

bool VFOutputState::isAllowed(VFInputState* inputState) const {
	return true;
}

bool VFOutputState::getValuesHaveChanged() const {
	return false;
}