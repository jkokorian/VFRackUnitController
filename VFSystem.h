#pragma once
class VFSystem
{
public:
	VFSystem::VFSystem()
	{
		
	}


	VFSystem::~VFSystem()
	{
	}

	int VFSystem::gotoState(int state)
	{
		const VFOutputState* targetState;
		switch (state)
		{
		case STOPSTATE:
			targetState = &_STOPSTATE;
			break;
		case FLOWSTATE:
			targetState = &_FLOWSTATE;
			break;
		case PUMPSTATE:
			targetState = &_PUMPSTATE;
			break;
		case PURGESTATE:
			targetState = &_PURGESTATE;
		case VENTSTATE:
			targetState = &_VENTSTATE;
			break;
		}

		if (targetState->isAllowed(&_inputState)) {
			targetState->apply();
		}
		return 0;
	}
	void setup(){}
	void update(){}
	
	

	int setPurgeFlowSetpoint(int setpoint){}
	int setVentFlowSetpoint(int setpoint){}
	int setPureArgonFlowSetpoint(int setpoint){}
	int setBubblerFlowSetpoint(int setpoint){}

private:
	VFInputState _inputState;
	VFOutputState* _outputState;

	const VentState _VENTSTATE;
	const PurgeState _PURGESTATE;
	const FlowState _FLOWSTATE;
	const PumpState _PUMPSTATE;
	const StopState _STOPSTATE;
	
};

