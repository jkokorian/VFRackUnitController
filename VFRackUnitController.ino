
#include <SPI.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include "FlowController.h"
#include "AD5263.h"

#define FIRMWARE_VERSION "Beta2"

#define SERIAL Serial1

//possible values for 'currentState'
#define STOPSTATE     0 //mfc's off, all valves closed, vacuum pump off
#define FLOWSTATE     1 //mfc's on, pre-chamber valves open, vacuum pump valve closed, vent valve open, vacuum pump off
#define PUMPSTATE     2 //mfc's off, all pre-chamber valves closed, vacuum pump valve open, vent valve closed, vacuum pump on
#define PURGESTATE    3 //pure argon mfc on, bubbler mfc off, bubbler valves closed, pure argon valve open, chamber in/outlets open, vent valve open, vacuum pump off
#define VENTSTATE     4 //pure argon mfc on, bubbler mfc off, bubbler valves closed, pure argon valve open, chamber in/outlets open, vent valve closed, vacuum pump off

//output port pin positions of valves
#define PURE_ARGON_VALVE_PIN             22
#define BUBBLER_INLET_VALVE_PIN          23
#define BUBBLER_OUTLET_VALVE_PIN         24
#define CHAMBER_INLET_VALVE_PIN          25
#define CHAMBER_OUTLET_VALVE_PIN         26
#define EXHAUST_VALVE_PIN                27
#define PURGE_VALVE_PIN                  28

//Flow controller input pin assignments
#define ACTUAL_BUBBLER_FLOW_PIN           0
#define ACTUAL_PURE_ARGON_FLOW_PIN        1
#define ACTUAL_CHAMBER_PRESSURE_PIN       2 //currently not in use

//Pressure Gauge controller relay inputs
#define PRESSURE_BELOW_THRESHOLD_PIN	 42
#define PRESSURE_GAUGE_ONLINE_PIN		 43


//LED indicators
#define CONNECTED_TO_PC_LED				LED_BUILTIN

//AD6253 quad digital potentiometer pin assignments
#define AD5263_CS						 49
#define AD5263_SD						 48
#define BUBBLER_FLOW_POT_CHANNEL          0
#define PURE_ARGON_FLOW_POT_CHANNEL       2
#define AD6253_REFERENCE_VOLTAGE		  5

#define MAXIMUM_FLOW_SETPOINT           400 //SCCM



FlowController bubblerFC(ACTUAL_BUBBLER_FLOW_PIN);
FlowController pureArgonFC(ACTUAL_PURE_ARGON_FLOW_PIN);

AD5263 dac(AD5263_CS, AD5263_SD);

#define AD5263VOLTAGE					  5 //Volts

float flowControllerKFactor = 1;

int pureArgonVentFlowSetpoint;
int pureArgonPurgeFlowSetpoint;

int previousState;
int currentState;

bool bubblerInletValveOpen;
bool bubblerOutletValveOpen;
bool pureArgonValveOpen;
bool chamberInletValveOpen;
bool chamberOutletValveOpen;
bool exhaustValveOpen;
bool purgeValveOpen;
bool vacuumPumpIsActive;

String serialCommand;
bool commandReceived;
bool manualStateChangeAllowed = true;
bool updatePhysicalOutputs = true;

bool chamberIsVacuum;
bool pressureGaugeIsActive;
bool chamberIsAtmospheric;

#define PC_HOST_VERIFICATION_TIMEOUT 3 //seconds
bool pcHostStatusVerified = false;
int pcHostStatusNotVerifiedTime = 0;


//----------------------------Accessors-------------------------


void setPureArgonVentFlowSetpoint(int value) {
	pureArgonVentFlowSetpoint = (value > MAXIMUM_FLOW_SETPOINT ? MAXIMUM_FLOW_SETPOINT : value);
}

void setPureArgonPurgeFlowSetpoint(int value) {
	pureArgonPurgeFlowSetpoint = (value > MAXIMUM_FLOW_SETPOINT ? MAXIMUM_FLOW_SETPOINT : value);
}


void setup() {

	//set valve control pins as outputs
	pinMode(BUBBLER_INLET_VALVE_PIN, OUTPUT);
	pinMode(BUBBLER_OUTLET_VALVE_PIN, OUTPUT);
	pinMode(PURE_ARGON_VALVE_PIN, OUTPUT);
	pinMode(CHAMBER_INLET_VALVE_PIN, OUTPUT);
	pinMode(CHAMBER_OUTLET_VALVE_PIN, OUTPUT);
	pinMode(EXHAUST_VALVE_PIN, OUTPUT);
	pinMode(PURGE_VALVE_PIN, OUTPUT);

	//set LED pins as outputs
	pinMode(CONNECTED_TO_PC_LED, OUTPUT);

	//set pressure gauge relay pins as inputs
	pinMode(PRESSURE_BELOW_THRESHOLD_PIN, INPUT);
	pinMode(PRESSURE_GAUGE_ONLINE_PIN, INPUT);


	//setup AD5263 on SPI bus
	dac.initialize();


	//initialize flowcontrollers
	bubblerFC.setKFactor(1);
	bubblerFC.setInputAdcFullScale(1023);
	bubblerFC.setSetpointAdcFullScale(255);
	bubblerFC.setFlowFullScale(500);

	pureArgonFC.setKFactor(1);
	pureArgonFC.setInputAdcFullScale(1023);
	pureArgonFC.setSetpointAdcFullScale(255);
	pureArgonFC.setFlowFullScale(500);

	//initialize communications
	SERIAL.begin(9600);
	serialCommand = "";

	//initialize pc host verification timer
	Timer3.initialize(1000000);
	Timer3.attachInterrupt(on_pcHostVerificationExpire);

	gotoStopState();
}



void loop() {


	//main state machine
	readPhysicalInputs();
	if (commandReceived) {
		executeCommand(serialCommand);
		serialCommand = "";
		commandReceived = false;
	}
	validateSystemState();
	if (updatePhysicalOutputs) {
		writePhysicalOutputs();
		updatePhysicalOutputs = false;
	}

}

/**
Reads all relevant analog and digital inputs and stores their values in the appropriate registers
**/
void readPhysicalInputs() {
	bubblerFC.getActualFlow();
	pureArgonFC.getActualFlow();
	readPressureGaugeControllerRelays();
}


/**
Writes all relevate analog and digital output ports
**/
void writePhysicalOutputs() {
	writeValves();
	writeBubblerFlowSetpoint();
	writePureArgonFlowSetpoint();
	writePcHostStatusVerified();
}


//----------------Port IO Methods------------------

/**
Reads analog pin 'ACTUAL_CHAMBER_PRESSURE' and stores its value in 'actualChamberPressure'
**/
void readPressureGaugeControllerRelays() {
	chamberIsVacuum = digitalRead(PRESSURE_BELOW_THRESHOLD_PIN);
	pressureGaugeIsActive = digitalRead(PRESSURE_GAUGE_ONLINE_PIN);
	chamberIsAtmospheric = (!chamberIsVacuum) && pressureGaugeIsActive;
}

/**
Turns the CONNECTED_TO_PC_LED on or off depending on pcHostStatusVerified
**/
void writePcHostStatusVerified() {
	digitalWrite(CONNECTED_TO_PC_LED, pcHostStatusVerified && pressureGaugeIsActive);
}

/**
Writes the value in 'bubblerFlowSetpoint' to AD5263 channel 0
**/
void writeBubblerFlowSetpoint() {
	dac.writeChannelValue(BUBBLER_FLOW_POT_CHANNEL, bubblerFC.getSetpointAdcValue());
}

/**
Writes the value in 'pureArgonFlowSetpoint' to AD5263 channel 1
**/
void writePureArgonFlowSetpoint() {
	dac.writeChannelValue(PURE_ARGON_FLOW_POT_CHANNEL, pureArgonFC.getSetpointAdcValue());
}



/**
Writes the values in the valve control registers to the corresponding digital output ports.
**/
void writeValves() {
	digitalWrite(BUBBLER_INLET_VALVE_PIN, bubblerInletValveOpen);
	digitalWrite(BUBBLER_OUTLET_VALVE_PIN, bubblerOutletValveOpen);
	digitalWrite(PURE_ARGON_VALVE_PIN, pureArgonValveOpen);
	digitalWrite(CHAMBER_INLET_VALVE_PIN, chamberInletValveOpen);
	digitalWrite(CHAMBER_OUTLET_VALVE_PIN, chamberOutletValveOpen);
	digitalWrite(EXHAUST_VALVE_PIN, exhaustValveOpen);
	digitalWrite(PURGE_VALVE_PIN, purgeValveOpen);
}


//-----------Communication Message Handling---------------


/**
Interrupt handler for Serial data that arrives. Adds incoming bytes to the 'serialCommand' string, and raises a flag 'commandReceived' when the 'END' character has been read.
**/
void serialEvent1() {
	while (SERIAL.available()) {
		// get the new byte:
		char inChar = (char)SERIAL.read();
		// add it to the inputString:

		// if the incoming character is a newline, set a flag
		// so the main loop can do something about it:
		if (inChar == '\n') {
			commandReceived = true;
		}
		else {
			serialCommand += inChar;
		}
	}

}

/**
Checks if there is a complete serial command waiting to be handled and handles it.
**/
void executeCommand(String command) {

	if (command == "BBFLOW?") {
		sendActualBubblerFlow();

	}
	else if (command == "PAFLOW?") {
		sendActualPureArgonFlow();

	}
	else if (command.startsWith("PAFLOWSP!") && command.length() > 9) {
		if (currentState == FLOWSTATE) {
			int value = command.substring(9).toInt();
			pureArgonFC.setSetpoint(value);
			updatePhysicalOutputs = true;
		}
		sendPureArgonFlowSetpoint();


	}
	else if (command.startsWith("BBFLOWSP!") && command.length() > 9) {
		if (currentState == FLOWSTATE) {
			int value = command.substring(9).toInt();
			bubblerFC.setSetpoint(value);
			updatePhysicalOutputs = true;
		}
		sendBubblerFlowSetpoint();
	}
	else if (command == "VALVES?") {
		sendValveStates();

	}
	else if (command == "STATE?") {
		sendSystemState();

	}
	else if (command == "PUMP?") {
		sendVacuumPumpState();

	}
	else if (command == "TEST") {
		sendDebugInfo();
	}
	else if (command == "VERSION?") {
		sendVersionInfo();
	}

	//the following commands are manual state change requests. Only the STOP command can be called when 'manualStateChangeAllowed' is false.
	else if (command == "STOP!"){
		gotoStopState();

	}

	else if (manualStateChangeAllowed) {
		if (command == "PUMP!") {
			gotoPumpState();

		}
		else if (command.startsWith("VENT!")) {
			//check if a setpoint value is included
			if (command.length() != 5) {
				int value = command.substring(5).toInt();
				setPureArgonVentFlowSetpoint(value);
			}

			if (chamberIsVacuum) {
				gotoVentState();
			}
			else {
				gotoPurgeState();
			}

		}
		else if (command == "FLOW!")  {
			if (chamberIsAtmospheric) {
				gotoFlowState();
			}
		}
		else if (command.startsWith("PURGE!")) {
			//check if a setpoint value is included
			if (command.length() > 6) {
				int value = command.substring(6).toInt();
				setPureArgonPurgeFlowSetpoint(value);
			}

			if (chamberIsAtmospheric) {
				gotoPurgeState();
			}
			else {
				gotoVentState();
			}
		}
		else {
			replySerial("E:UNKNOWN: " + command);

		}
	}

	pcHostStatusNotVerifiedTime = 0;
	pcHostStatusVerified = true;

}


void sendActualPureArgonFlow() {
	String reply = String("PAFLOW:") + String((int)pureArgonFC.getActualFlow());
	replySerial(reply);
}

void sendActualBubblerFlow() {
	String reply = String("BBFLOW:") + String((int)bubblerFC.getActualFlow());
	replySerial(reply);
}

void sendPureArgonFlowSetpoint() {
	String reply = String("PAFLOWSP:") + String((int)pureArgonFC.getSetpoint());
	replySerial(reply);
}


void sendBubblerFlowSetpoint() {
	String reply = String("BBFLOWSP:") + String((int)bubblerFC.getSetpoint());
	replySerial(reply);
}


void sendValveStates() {
	String reply = String("VALVES:");
	reply += (pureArgonValveOpen ? "1" : "0");
	reply += (bubblerInletValveOpen ? "1" : "0");
	reply += (bubblerOutletValveOpen ? "1" : "0");
	reply += (chamberInletValveOpen ? "1" : "0");
	reply += (chamberOutletValveOpen ? "1" : "0");
	reply += (exhaustValveOpen ? "1" : "0");
	reply += (purgeValveOpen ? "1" : "0");

	replySerial(reply);

}


void sendSystemState() {
	String reply = String("STATE:");
	reply += String(currentState);
	replySerial(reply);
}

void sendVacuumPumpState() {
	String reply = String("PUMP:") + (vacuumPumpIsActive ? String("1") : String("0"));
	replySerial(reply);
}

void sendDebugInfo() {
	replySerial(String(manualStateChangeAllowed) + String(chamberIsVacuum) + String(pressureGaugeIsActive) + String(currentState));
}

void sendVersionInfo() {
	String reply = String("VERSION:") + String("FIRMWARE ") + String(FIRMWARE_VERSION);
	replySerial(reply);

}

/**
Sends a message to SERIAL and terminates it with a newline (\n) character.
**/
void replySerial(String message) {
	SERIAL.print(message + "\n");
}

//---------------------System State Changes---------------------------

/**
In the STOP state, all valves are closed and the vacuum pump is inactive.
**/
void gotoStopState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.deactivate();
	pureArgonFC.setSetpoint(0);
	bubblerFC.setSetpoint(0);

	pureArgonValveOpen = false;
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	chamberInletValveOpen = false;
	chamberOutletValveOpen = false;
	exhaustValveOpen = false;
	purgeValveOpen = false;

	//raise flags
	previousState = currentState;
	currentState = STOPSTATE;
	updatePhysicalOutputs = true;

}

/**
In the PUMP state, the vacuum pump is active and evacuates the chamber. The pre-chamber gas system remains at atmospheric pressure.
**/
void gotoPumpState() {

	//set outputs
	vacuumPumpIsActive = true;

	dac.deactivate();
	pureArgonFC.setSetpoint(0);
	bubblerFC.setSetpoint(0);

	pureArgonValveOpen = false;
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	chamberInletValveOpen = false;
	chamberOutletValveOpen = true;
	exhaustValveOpen = false;
	purgeValveOpen = false;

	//raise flags
	previousState = currentState;
	currentState = PUMPSTATE;
	updatePhysicalOutputs = true;



}

/**
In the VENT state, the chamber is at low pressure and is being filled with pure argon or nitrogen.
**/
void gotoVentState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.activate();
	pureArgonFC.setSetpoint(MAXIMUM_FLOW_SETPOINT);
	bubblerFC.setSetpoint(0);

	pureArgonValveOpen = true;
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	exhaustValveOpen = false;
	purgeValveOpen = true;


	//raise flags
	previousState = currentState;
	currentState = VENTSTATE;
	updatePhysicalOutputs = true;


}

/**
In the FLOW state, all components are at atmospheric pressure. Both the bubbler MFC and the pure argon MFC operate at their respective setpoints.
**/
void gotoFlowState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.activate();

	pureArgonValveOpen = true;
	bubblerInletValveOpen = true;
	bubblerOutletValveOpen = true;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	exhaustValveOpen = true;
	purgeValveOpen = false;


	//raise flags
	previousState = currentState;
	currentState = FLOWSTATE;
	updatePhysicalOutputs = true;


}

/**
In the PURGE state, all components are at atmospheric pressure. The chamber is being purged with pure argon and or nitrogen.
**/
void gotoPurgeState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.activate();
	pureArgonFC.setSetpoint(MAXIMUM_FLOW_SETPOINT);
	bubblerFC.setSetpoint(0);

	pureArgonValveOpen = true;
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	purgeValveOpen = true;
	exhaustValveOpen = true;

	//raise flags
	previousState = currentState;
	currentState = PURGESTATE;
	updatePhysicalOutputs = true;

}

/**
Callback method for pcHostStatusVerification timer.
When the timer expires, 'pcHostStatusVerified' is set to false
**/
void on_pcHostVerificationExpire() {
	if (pcHostStatusNotVerifiedTime < PC_HOST_VERIFICATION_TIMEOUT) {
		pcHostStatusNotVerifiedTime++;
	}
	else {
		pcHostStatusVerified = false;
		pcHostStatusNotVerifiedTime = 0;
	}
}


//-----------------------Emergency Handling---------------------

/**
Checks all input values. If any inconsistencies are found, the system is immediately transferred to the STOP state.
**/
void validateSystemState() {

	if (!pcHostStatusVerified || !pressureGaugeIsActive) {
		manualStateChangeAllowed = false;
		gotoStopState();
	}
	else {
		manualStateChangeAllowed = true;
		
		//if the chamber is in the ventstate and atmospheric pressure has been reached, go to the purge state
		if (currentState == VENTSTATE && chamberIsAtmospheric) {
			gotoPurgeState();
		}
		
		//if the chamber is in the purge state and somehow the pressure drops, go the the vent state
		else if (currentState == PURGESTATE && chamberIsVacuum) {
			gotoVentState();
		}
	}
}




