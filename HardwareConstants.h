#define SERIAL Serial1

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