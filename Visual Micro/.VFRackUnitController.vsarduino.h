#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
//Board = Arduino Mega 2560 or Mega ADK
#define __AVR_ATmega2560__
#define ARDUINO 103
#define ARDUINO_MAIN
#define __AVR__
#define __avr__
#define F_CPU 16000000L
#define __cplusplus
#define __inline__
#define __asm__(x)
#define __extension__
#define __ATTR_PURE__
#define __ATTR_CONST__
#define __inline__
#define __asm__ 
#define __volatile__

#define __builtin_va_list
#define __builtin_va_start
#define __builtin_va_end
#define __DOXYGEN__
#define __attribute__(x)
#define NOINLINE __attribute__((noinline))
#define prog_void
#define PGM_VOID_P int
            
typedef unsigned char byte;
extern "C" void __cxa_pure_virtual() {;}

void setPureArgonVentFlowSetpoint(int value);
void setPureArgonPurgeFlowSetpoint(int value);
//
//
void readPhysicalInputs();
void writePhysicalOutputs();
void setPcHostStatusVerified(bool verified);
void writeBubblerFlowSetpoint();
void writePureArgonFlowSetpoint();
void readActualChamberPressure();
void writeValves();
void serialEvent1();
void executeCommand(String command);
void sendActualPureArgonFlow();
void sendActualBubblerFlow();
void sendPureArgonFlowSetpoint();
void sendBubblerFlowSetpoint();
void sendValveStates();
void sendSystemState();
void sendVacuumPumpState();
void sendDebugInfo();
void sendVersionInfo();
void replySerial(String message);
void gotoStopState();
void gotoPumpState();
void gotoVentState();
void gotoFlowState();
void gotoPurgeState();
void on_pcHostVerificationExpire();
void checkForEmergency();
void handleEmergency();

#include "D:\Portable Software\arduino-1.0.3\hardware\arduino\cores\arduino\arduino.h"
#include "D:\Portable Software\arduino-1.0.3\hardware\arduino\variants\mega\pins_arduino.h" 
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFRackUnitController.ino"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\AD5263.cpp"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\AD5263.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\FlowController.cpp"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\FlowController.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\FlowState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\HardwareConstants.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\PumpState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\PurgeState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\State.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\StopState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFInputState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFOutputState.cpp"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFOutputState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFSystem.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VFSystemConstants.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\VentState.h"
#include "D:\jkokorian\My Documents\_Delft\PhD\Projects\Valve and Flow Controller Rack Unit\Arduino\VFRackUnitController\resource.h"
#endif
