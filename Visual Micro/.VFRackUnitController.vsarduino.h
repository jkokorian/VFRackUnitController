/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Mega 2560 or Mega ADK, Platform=avr, Package=arduino
*/

#ifndef _VSARDUINO_H_
#define _VSARDUINO_H_
#define __AVR_ATmega2560__
#define ARDUINO 104
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
void setPureArgonFlushFlowSetpoint(int value);
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
void sendPureArgonWorkingSetpoint();
void sendPureArgonFlowStateSetpoint();
void sendPureArgonVentStateSetpoint();
void sendPureArgonFlushStateSetpoint();
void sendBubblerWorkingSetpoint();
void sendBubblerFlowStateSetpoint();
void sendPressure();
void sendValveStates();
void sendSystemState();
void sendVacuumPumpState();
void sendDebugInfo();
void sendVersionInfo();
void replySerial(String message);
void gotoStopState();
void gotoPumpState();
bool isPumpStateAllowed();
bool isFlowStateAllowed();
void gotoVentState();
void gotoFlowState();
void gotoFlushState();
void on_pcHostVerificationExpire();
void checkForEmergency();
void handleEmergency();

#include "D:\arduino-1.0.4\hardware\arduino\cores\arduino\arduino.h"
#include "D:\arduino-1.0.4\hardware\arduino\variants\mega\pins_arduino.h" 
#include "C:\Users\ODMS-admin\Documents\Visual Studio 2010\Projects\VFRackUnitController\VFRackUnitController.ino"
#include "C:\Users\ODMS-admin\Documents\Visual Studio 2010\Projects\VFRackUnitController\AD5263.cpp"
#include "C:\Users\ODMS-admin\Documents\Visual Studio 2010\Projects\VFRackUnitController\AD5263.h"
#include "C:\Users\ODMS-admin\Documents\Visual Studio 2010\Projects\VFRackUnitController\FlowController.cpp"
#include "C:\Users\ODMS-admin\Documents\Visual Studio 2010\Projects\VFRackUnitController\FlowController.h"
#endif