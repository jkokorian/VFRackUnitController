
#include <SPI.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include "FlowController.h"
#include "AD5263.h"

#define FIRMWARE_VERSION "Beta1"

#define SERIAL Serial1

//possible values for 'currentState'
#define STOPSTATE     0 //mfc's off, all valves closed, vacuum pump off
#define FLOWSTATE     1 //mfc's on, pre-chamber valves open, vacuum pump valve closed, vent valve open, vacuum pump off
#define PUMPSTATE     2 //mfc's off, all pre-chamber valves closed, vacuum pump valve open, vent valve closed, vacuum pump on
#define FLUSHSTATE    3 //pure argon mfc on, bubbler mfc off, bubbler valves closed, pure argon valve open, chamber in/outlets open, vent valve open, vacuum pump off
#define VENTSTATE     4 //pure argon mfc on, bubbler mfc off, bubbler valves closed, pure argon valve open, chamber in/outlets open, vent valve closed, vacuum pump off

//output port pin positions of valves
#define PURE_ARGON_VALVE_PIN             22
#define BUBBLER_INLET_VALVE_PIN          23
#define BUBBLER_OUTLET_VALVE_PIN         24
#define CHAMBER_INLET_VALVE_PIN          25
#define CHAMBER_OUTLET_VALVE_PIN         26
#define VENT_VALVE_PIN                   27
#define PUMP_VALVE_PIN                   28

//Flow controller input pin assignments
#define ACTUAL_BUBBLER_FLOW_PIN           0
#define ACTUAL_PURE_ARGON_FLOW_PIN        1
#define ACTUAL_CHAMBER_PRESSURE_PIN       2

//LED indicators
#define CONNECTED_TO_PC_LED				LED_BUILTIN

//AD6253 quad digital potentiometer pin assignments
#define AD5263_CS						 49
#define AD5263_SD						 48
#define BUBBLER_FLOW_POT_CHANNEL          0
#define PURE_ARGON_FLOW_POT_CHANNEL       2
#define AD6253_REFERENCE_VOLTAGE		  5

#define MAXIMUM_FLOW_SETPOINT           500 //SCCM
#define VACUUM_PRESSURE_THRESHOLD       1000

FlowController bubblerFC(ACTUAL_BUBBLER_FLOW_PIN);
FlowController pureArgonFC(ACTUAL_PURE_ARGON_FLOW_PIN);

AD5263 dac(AD5263_CS, AD5263_SD);

#define AD5263VOLTAGE					  5 //Volts

float flowControllerKFactor = 1;

int pureArgonVentStateSetpoint;
int pureArgonFlushStateSetpoint;
int pureArgonFlowStateSetpoint;
int bubblerFlowStateSetpoint;

int actualChamberPressure;

int currentState;
char previousState;

bool bubblerInletValveOpen;
bool bubblerOutletValveOpen;
bool pureArgonValveOpen;
bool chamberInletValveOpen;
bool chamberOutletValveOpen;
bool ventValveOpen;
bool pumpValveOpen;
bool vacuumPumpIsActive;

String serialCommand;
bool commandReceived;
bool emergency;
bool manualStateChangeAllowed = true;
bool updatePhysicalOutputs = true;

bool chamberIsDirty;
bool chamberIsVacuum;

#define PC_HOST_VERIFICATION_TIMEOUT 20 //seconds
bool pcHostStatusVerified = false;
int pcHostStatusNotVerifiedTime = 0;


//----------------------------Accessors-------------------------


void setPureArgonVentFlowSetpoint(int value) {
	pureArgonVentStateSetpoint = (value > MAXIMUM_FLOW_SETPOINT ? MAXIMUM_FLOW_SETPOINT : value);
}

void setPureArgonFlushFlowSetpoint(int value) {
	pureArgonFlushStateSetpoint = (value > MAXIMUM_FLOW_SETPOINT ? MAXIMUM_FLOW_SETPOINT : value);
}


void setup() {
	
	//set valve control pins as outputs
	pinMode(BUBBLER_INLET_VALVE_PIN, OUTPUT);
	pinMode(BUBBLER_OUTLET_VALVE_PIN, OUTPUT);
	pinMode(PURE_ARGON_VALVE_PIN, OUTPUT);
	pinMode(CHAMBER_INLET_VALVE_PIN, OUTPUT);
	pinMode(CHAMBER_OUTLET_VALVE_PIN, OUTPUT);
	pinMode(VENT_VALVE_PIN, OUTPUT);
	pinMode(PUMP_VALVE_PIN, OUTPUT);

	//set LED pins as outputs
	pinMode(CONNECTED_TO_PC_LED,OUTPUT);


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

	

}



void loop() {
	
	
	//main state machine
	readPhysicalInputs();
	checkForEmergency();
	if (emergency) {
		handleEmergency();

	} else if (commandReceived) {
		executeCommand(serialCommand);
		serialCommand = "";
		commandReceived = false;
	}

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
	readActualChamberPressure();
}


/**
Writes all relevate analog and digital output ports
**/
void writePhysicalOutputs() {
  writeValves();
  writeBubblerFlowSetpoint();
  writePureArgonFlowSetpoint();
}


void setPcHostStatusVerified(bool verified) {
	pcHostStatusNotVerifiedTime = 0;
	if (verified) {
		pcHostStatusVerified = true;
		digitalWrite(CONNECTED_TO_PC_LED, HIGH);
	} else {
		pcHostStatusVerified = false;
		executeCommand("STOP!");
		digitalWrite(CONNECTED_TO_PC_LED, LOW);
	}
	
}


//----------------Port IO Methods------------------

/**
Writes the value in 'bubblerFlowSetpoint' to AD5263 channel 0
**/
void writeBubblerFlowSetpoint() {
	dac.writeChannelValue(BUBBLER_FLOW_POT_CHANNEL,bubblerFC.getSetpointAdcValue());
}

/**
Writes the value in 'pureArgonFlowSetpoint' to AD5263 channel 1
**/
void writePureArgonFlowSetpoint() {
	dac.writeChannelValue(PURE_ARGON_FLOW_POT_CHANNEL,pureArgonFC.getSetpointAdcValue());
}


/**
Reads analog pin 'ACTUAL_CHAMBER_PRESSURE' and stores its value in 'actualChamberPressure'
**/
void readActualChamberPressure() {
	actualChamberPressure = analogRead(ACTUAL_CHAMBER_PRESSURE_PIN);
	if (actualChamberPressure <= VACUUM_PRESSURE_THRESHOLD) {
		chamberIsVacuum = true;
	} else {
		chamberIsVacuum = false;
	}
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
	digitalWrite(VENT_VALVE_PIN, ventValveOpen);
	digitalWrite(PUMP_VALVE_PIN, pumpValveOpen);
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
	} else {
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
	
	} else if (command == "PAFLOW?") {
		sendActualPureArgonFlow();

	} else if (command.startsWith("PAFLOWSP!") && command.length() > 9) {
		pureArgonFlowStateSetpoint = command.substring(9).toInt();
		if (currentState == FLOWSTATE) {
			pureArgonFC.setSetpoint(pureArgonFlowStateSetpoint);
			updatePhysicalOutputs = true;
		}
		sendPureArgonFlowStateSetpoint();
	
	} else if (command.startsWith("PAVENTSP!") && command.length() > 9) {
		pureArgonVentStateSetpoint = command.substring(9).toInt();
		if (currentState == VENTSTATE) {
			pureArgonFC.setSetpoint(pureArgonVentStateSetpoint);
			updatePhysicalOutputs = true;
		}
		sendPureArgonVentStateSetpoint();

	} else if (command.startsWith("PAFLUSHSP!") && command.length() > 10) {
		pureArgonFlushStateSetpoint = command.substring(10).toInt();
		if (currentState == FLUSHSTATE) {
			pureArgonFC.setSetpoint(pureArgonFlushStateSetpoint);
			updatePhysicalOutputs = true;
		}
		sendPureArgonFlushStateSetpoint();

	} else if (command.startsWith("BBFLOWSP!") && command.length() > 9) {
		bubblerFlowStateSetpoint = command.substring(9).toInt();
		if (currentState == FLOWSTATE) {
			bubblerFC.setSetpoint(bubblerFlowStateSetpoint);
			updatePhysicalOutputs = true;
		}
		sendBubblerFlowStateSetpoint();

	} else if (command == "VALVES?") {
		sendValveStates();

	} else if (command == "STATE?") {
		sendSystemState();

	} else if (command == "PUMP?") {
		sendVacuumPumpState();

	} else if (command == "TEST") {
		sendDebugInfo();

	} else if (command == "VERSION?") {
		sendVersionInfo();

	} else if (command == "PRESSURE?") {
		sendPressure();

	} else if (command == "PAFLOWSP?") {
		sendPureArgonFlowStateSetpoint();
	
	} else if (command == "BBFLOWSP?") {
		sendBubblerFlowStateSetpoint();

	} else if (command == "PAVENTSP?") {
		sendPureArgonVentStateSetpoint();
	
	} else if (command == "PAWSP?") {
		sendPureArgonWorkingSetpoint();
	
	} else if (command == "BBWSP?") {
		sendBubblerWorkingSetpoint();
	
	} else if (command == "PAFLUSHSP?") {
		sendPureArgonFlushStateSetpoint();
	}
	
	//the following commands are manual state change requests. Only the STOP command can be called when 'manualStateChangeAllowed' is false.
	else if (command == "STOP!"){
		gotoStopState();
		
	}

	else if (manualStateChangeAllowed) {
		if (command == "PUMP!") {
			if (isPumpStateAllowed()) {
				gotoPumpState();
			}

		} else if (command == "VENT!") {
			gotoVentState();

		} else if (command == "FLOW!") {
			if (isFlowStateAllowed()) {
				gotoFlowState();
			}

		} else if (command == "FLUSH!") {
			gotoFlushState();

		} else {
			replySerial("E:UNKNOWN: " + command);
		}
	}

	setPcHostStatusVerified(true);
	
}


void sendActualPureArgonFlow() {
	String reply = String("PAFLOW:") + String((int)pureArgonFC.getActualFlow());
	replySerial(reply);
}

void sendActualBubblerFlow() {
	String reply = String("BBFLOW:") + String((int)bubblerFC.getActualFlow());
	replySerial(reply);
}

void sendPureArgonWorkingSetpoint() {
	String reply = String("PAWSP:") + String((int)pureArgonFC.getSetpoint());
	replySerial(reply);
}

void sendPureArgonFlowStateSetpoint() {
	String reply = String("PAFLOWSP:") + String((int)pureArgonFlowStateSetpoint);
	replySerial(reply);
}

void sendPureArgonVentStateSetpoint() {
	String reply = String("PAVENTSP:") + String((int)pureArgonVentStateSetpoint);
	replySerial(reply);
}

void sendPureArgonFlushStateSetpoint() {
	String reply = String("PAFLUSHSP:") + String((int)pureArgonFlushStateSetpoint);
	replySerial(reply);
}

void sendBubblerWorkingSetpoint() {
	String reply = String("BBWSP:") + String((int)bubblerFC.getSetpoint());
	replySerial(reply);
}

void sendBubblerFlowStateSetpoint() {
	String reply = String("BBFLOWSP:") + String((int)bubblerFlowStateSetpoint);
	replySerial(reply);
}

void sendPressure() {
	String reply = String("PRESSURE:") + String((int)actualChamberPressure);
	replySerial(reply);
}

void sendValveStates() {
	String reply = String("VALVES:");
	reply += (pureArgonValveOpen ? "1" : "0");
	reply += (bubblerInletValveOpen ? "1" : "0");
	reply += (bubblerOutletValveOpen ? "1" : "0");
	reply += (chamberInletValveOpen ? "1" : "0");
	reply += (chamberOutletValveOpen? "1" : "0");
	reply += (ventValveOpen ? "1" : "0");
	reply += (pumpValveOpen ? "1" : "0");
	
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
	
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	pureArgonValveOpen = false;
	chamberInletValveOpen = false;
	chamberOutletValveOpen = false;
	pumpValveOpen = false;
	ventValveOpen = false;
	
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
	
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	pureArgonValveOpen = false;
	chamberInletValveOpen = false;
	chamberOutletValveOpen = true;
	pumpValveOpen = true;
	ventValveOpen = false;

	//raise flags
	previousState = currentState;
	currentState = PUMPSTATE;
	updatePhysicalOutputs = true;
	
}

bool isPumpStateAllowed() {
	return true;
}

bool isFlowStateAllowed() {
	if (chamberIsVacuum) {
		return false;
	} else {
		return true;
	}
}

/**
In the VENT state, the chamber is at low pressure and is being filled with pure argon.
**/
void gotoVentState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.activate();
	pureArgonFC.setSetpoint(pureArgonVentStateSetpoint);
	bubblerFC.setSetpoint(0);
	
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	pureArgonValveOpen = true;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	pumpValveOpen = false;
	ventValveOpen = false;


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
	pureArgonFC.setSetpoint(pureArgonFlowStateSetpoint);
	bubblerFC.setSetpoint(bubblerFlowStateSetpoint);
	dac.activate();

	bubblerInletValveOpen = true;
	bubblerOutletValveOpen = true;
	pureArgonValveOpen = true;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	pumpValveOpen = false;
	ventValveOpen = true;


	//raise flags
	previousState = currentState;
	currentState = FLOWSTATE;
	updatePhysicalOutputs = true;


}

/**
In the FLUSH state, all components are at atmospheric pressure. The chamber is being flushed with pure argon.
**/
void gotoFlushState() {

	//set outputs
	vacuumPumpIsActive = false;

	dac.activate();
	pureArgonFC.setSetpoint(pureArgonFlushStateSetpoint);
	bubblerFC.setSetpoint(0);
	
	bubblerInletValveOpen = false;
	bubblerOutletValveOpen = false;
	pureArgonValveOpen = true;
	chamberInletValveOpen = true;
	chamberOutletValveOpen = true;
	pumpValveOpen = false;
	ventValveOpen = true;

	//raise flags
	previousState = currentState;
	currentState = FLUSHSTATE;
	updatePhysicalOutputs = true;

}

/**
Callback method for pcHostStatusVerification timer.
When the timer expires, 'pcHostStatusVerified' is set to false
**/
void on_pcHostVerificationExpire() {
	if (pcHostStatusNotVerifiedTime < PC_HOST_VERIFICATION_TIMEOUT) {
		pcHostStatusNotVerifiedTime++;
	} else {
		setPcHostStatusVerified(false);
	}
}


//-----------------------Emergency Handling---------------------

/**
Checks all input values. If any inconsistencies are found, the system is immediately transferred to the STOP state.
**/
void checkForEmergency() {
	//TODO: stub
	emergency = false;
}

/**
This code executes when an emergency has been detected
**/
void handleEmergency() {
	executeCommand("STOP!");
}




