#include <Servo.h>
#include <PS2X_lib.h>  //version 1.6 or newer

// Set the connection pins for the PS2 controller
#define PS2_DAT        2  
#define PS2_CMD        4 
#define PS2_SEL        7
#define PS2_CLK        8


#define pressures   false
#define rumble      false

PS2X ps2x; 

// Define the servos for the motion of the robot arm.
Servo ROT_servo;
Servo LEAN_servo;
Servo ELEV_servo;
Servo JAWS_servo;

int ROT_angle;
int LEAN_angle;
int ELEV_angle;
int JAWS_angle;

int ROT_vel = 2;
int LEAN_vel = 1;
int ELEV_vel = 1;
int JAWS_vel = 10;

const int ELEV_max = 110;
const int ELEV_max_for_large_LEAN = 95; // A lower maximum for ELEVATION if LEAN is large
// A variable lower bound for ELEV based on the value of LEAN
const float ELEV_LB_slope = float(float(50-70)/float(105-80));
const float ELEV_LB_offset = float(70-80*ELEV_LB_slope);
const int LEAN_max = 135;
const int LEAN_max_for_large_ELEV = 115; // A lower maximum for LEAN if ELEV is large
const float LEAN_LB_slope = 1/ELEV_LB_slope;
const float LEAN_LB_offset = float(105-50*LEAN_LB_slope);
const int LEAN_min = 60;
const int ELEV_min = 30;

const int JAWS_max = 120;
const int ROT_max = 180;
const int ROT_min = 0;
const int JAWS_min = 28;

int LYval;
int RYval;
int RXval;

int error = 0;
byte type = 0;
byte vibrate = 0;

void setup(){
 
  // The setup procedure for the PS2 controller (from the PS2X library examples)
  Serial.begin(57600);
  
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
	if (pressures)
	  Serial.println("true ");
	else
	  Serial.println("false");
	Serial.print("rumble = ");
	if (rumble)
	  Serial.println("true)");
	else
	  Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
//  Serial.print(ps2x.Analog(1), HEX);
  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
	case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
   }

   // Move the servos to the initial positions
   ROT_angle = 90;
   LEAN_angle = 90;
   ELEV_angle = 90;
   JAWS_angle = 90;

   ROT_servo.attach(3);
   LEAN_servo.attach(5);
   ELEV_servo.attach(6);
   JAWS_servo.attach(9);

}

void loop() {
  if(error == 1) 
    return; 
  
  ps2x.read_gamepad(false, vibrate); 

  // STICK coordinates: Y: 0(UP)-127(CENTER)-255(DOWN)
  // 0-40 -> increase with 3x velocity
  // 41-124 -> increase with 1x velocity
  // 130-214 -> decrease with 1x velocity
  // 215-255 -> decrease with 3x velocity

  // Right stick Y-axis: Elevation
  RYval = ps2x.Analog(PSS_RY);
  //Serial.println(RYval, DEC);
  if (RYval < 41) {
    if ((ELEV_angle < ELEV_max && LEAN_angle < LEAN_max_for_large_ELEV) || ELEV_angle < ELEV_max_for_large_LEAN) {
      ELEV_angle += 3*ELEV_vel;
      ELEV_servo.write(ELEV_angle);
      Serial.print("Elevation angle: ");
      Serial.print(ELEV_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RYval < 124) {
    if ((ELEV_angle < ELEV_max && LEAN_angle < LEAN_max_for_large_ELEV) || ELEV_angle < ELEV_max_for_large_LEAN) {
      ELEV_angle += ELEV_vel;
      ELEV_servo.write(ELEV_angle);
      Serial.print("Elevation angle: ");
      Serial.print(ELEV_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RYval > 129 && RYval < 215) {
    if ((ELEV_angle > ELEV_min) && (ELEV_angle > (ELEV_LB_slope*float(LEAN_angle)+ELEV_LB_offset))) {
    // 70+(t-80)*(50-70)/(105-80);
    //if (ELEV_angle > ELEV_min) {
      ELEV_angle -= ELEV_vel;
      ELEV_servo.write(ELEV_angle);
      Serial.print("Elevation angle: ");
      Serial.print(ELEV_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RYval > 214) {
    if ((ELEV_angle > ELEV_min) && (ELEV_angle > (ELEV_LB_slope*float(LEAN_angle)+ELEV_LB_offset))) {
 //   if (ELEV_angle > ELEV_min) {
      ELEV_angle -= 3*ELEV_vel;
      ELEV_servo.write(ELEV_angle);
      Serial.print("Elevation angle: ");
      Serial.print(ELEV_angle,DEC);
      Serial.print("\n");
    }
  }

  // Left stick Y-axis: Lean
  LYval = ps2x.Analog(PSS_LY);
  //Serial.println(LYval, DEC);
  if (LYval < 41) {
    if ((LEAN_angle < LEAN_max && ELEV_angle < ELEV_max_for_large_LEAN) || LEAN_angle < LEAN_max_for_large_ELEV) {
      LEAN_angle += 3*LEAN_vel;
      LEAN_servo.write(LEAN_angle);
      Serial.print("Lean angle: ");
      Serial.print(LEAN_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (LYval < 124) {
    if ((LEAN_angle < LEAN_max && ELEV_angle < ELEV_max_for_large_LEAN) || LEAN_angle < LEAN_max_for_large_ELEV) {
      LEAN_angle += LEAN_vel;
      LEAN_servo.write(LEAN_angle);
      Serial.print("Lean angle: ");
      Serial.print(LEAN_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (LYval > 129 && LYval < 215) {
    if ((LEAN_angle > LEAN_min) &&  (LEAN_angle > (LEAN_LB_slope*float(ELEV_angle)+LEAN_LB_offset))) {
    // if (LEAN_angle > LEAN_min) {
      LEAN_angle -= LEAN_vel;
      LEAN_servo.write(LEAN_angle);
      Serial.print("Lean angle: ");
      Serial.print(LEAN_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (LYval > 214) {
    if ((LEAN_angle > LEAN_min) &&  (LEAN_angle > (LEAN_LB_slope*float(ELEV_angle)+LEAN_LB_offset))) {
//    if (LEAN_angle > LEAN_min) {
      LEAN_angle -= 3*LEAN_vel;
      LEAN_servo.write(LEAN_angle);
      Serial.print("Lean angle: ");
      Serial.print(LEAN_angle,DEC);
      Serial.print("\n");
    }
  }

  // RIGHT stick X-axis: Base rotation
  RXval = ps2x.Analog(PSS_RX);
  //Serial.println(RXval, DEC);
  if (RXval < 41) {
    if (ROT_angle > ROT_min) {
      ROT_angle -= 3*ROT_vel;
      ROT_servo.write(ROT_angle);
      Serial.print("Rotation angle: ");
      Serial.print(ROT_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RXval < 124) {
    if (ROT_angle > ROT_min) {
      ROT_angle -= ROT_vel;
      ROT_servo.write(ROT_angle);
      Serial.print("Rotation angle: ");
      Serial.print(ROT_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RXval > 129 && RXval < 215) {
    if (ROT_angle < ROT_max) {
      ROT_angle += ROT_vel;
      ROT_servo.write(ROT_angle);
      Serial.print("Rotation angle: ");
      Serial.print(ROT_angle,DEC);
      Serial.print("\n");
    }
  }
  else if (RXval > 214) {
    if (ROT_angle < ROT_max) {
      ROT_angle += 3*ROT_vel;
      ROT_servo.write(ROT_angle);
      Serial.print("Rotation angle: ");
      Serial.print(ROT_angle,DEC);
      Serial.print("\n");
    }
  }

  // Jaws open and close, R1 to the close, L1 to the open
  if(ps2x.Button(PSB_R1))
    //Serial.println("R1 pressed");
    if (JAWS_angle >= (JAWS_min+JAWS_vel)) {
      JAWS_angle -= JAWS_vel;
      JAWS_servo.write(JAWS_angle);
      Serial.print("Jaws angle: ");
      Serial.print(JAWS_angle,DEC);
      Serial.print("\n");
    }
  if(ps2x.Button(PSB_L1))
    //Serial.println("L1 pressed");
    if (JAWS_angle <= (JAWS_max-JAWS_vel)) {
      JAWS_angle += JAWS_vel;
      JAWS_servo.write(JAWS_angle);
      Serial.print("Jaws angle: ");
      Serial.print(JAWS_angle,DEC);
      Serial.print("\n");
    }

  delay(50);  
}
