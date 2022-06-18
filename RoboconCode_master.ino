#include <PS2X_lib.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();


PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

//13, 11, 10, 12 use for receiver

//Define direction state of running motor
#define GO_FORWARD 1
#define GO_BACKWARD -1
#define STOP_GO 0

#define R_IS_lift 40
#define L_IS_lift 41
#define R_EN_lift 42
#define L_EN_lift 43
#define R_PWM_lift 45
#define L_PWM_lift 44
#define lift_speed_default 200
#define lift_speed_default_slow 100

//define the state of joystick
int js_left_x = 128;
int js_left_y = 127;
int js_right_x = 128;
int js_right_y = 127;

//Catching ball state
#define SQUEEZE_BALL 1
#define ROLL_BALL 0

//define lift_catch dir and speed
int lift_catch_dir = 0;
int lift_speed = 0;

//initialize alternate state
int catchState = SQUEEZE_BALL;

//pick the ball state
#define SQUEEZE_UP 1
#define SQUEEZE_DOWN 0

//initialize alternate state
int squeeze_state = SQUEEZE_UP;

//Define rotate squeeze ball
#define SQUEEZE_ROTATE_T 1
#define SQUEEZE_ROTATE_F 0

//initialize alternate rotate squeeze state
int squeeze_rotate_state = SQUEEZE_ROTATE_T;

//define pin for lifting lagori
#define IN1_LIFT_LAGO 51
#define IN2_LIFT_LAGO 50
#define PWM_LIFT_LAGO 49

int liftDir = 0;
int stateDir = 0;

// motor catch
int rollDir = 0;
int right_state = 0;
int left_state = 0;

#define GO_UP 1;
#define GO_DOWN -1

#define GO_RIGHT_LEFT 400
#define GO_LEFT_RIGHT -400

//define pin for motor squeeze lagori
#define IN1_SQUEEZE_LAGO 47
#define IN2_SQUEEZE_LAGO 48
#define PWM_SQUEEZE_LAGO 46
#define SQUEEZE_DEFAULT_SPEED 50

#define DEFAULT_SPEED 140
#define FASTER 110
#define SLOWER -60

//initialize velocoty
int velo = 0;
int additional_speed = 0;

int squeeze_lago_speed = 0;
int catch_state = 0;

int mode_speed = 1;

#define SPEED_RUN_LAGO 50

void setup() {
  Serial.begin(9600);

  //Code for PS2 error to determind type of controller
  do {
    delay(1000);
    error = ps2x.config_gamepad(13, 11, 10, 12, true, true);//GamePad(clock, command, attention, data, Pressures?, Rumble?)
  }while(error ==1);
  if (error == 0) {
    Serial.println("Found Controller, configured successful");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Go to www.billporter.info for updates and to report bugs.");
  }
  else if (error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
  else if (error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  else if (error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.println("Unknown Controller type");
      break;
    case 1:
      Serial.println("DualShock Controller Found");
      break;
    case 2:
      Serial.println("GuitarHero Controller Found");
      break;
  }
  
  // for catching lift motor
  pinMode(R_IS_lift, OUTPUT);
  pinMode(R_EN_lift, OUTPUT);
  pinMode(L_IS_lift, OUTPUT);
  pinMode(L_EN_lift, OUTPUT);
  digitalWrite(R_IS_lift, LOW);
  digitalWrite(L_IS_lift, LOW);
  digitalWrite(R_EN_lift, HIGH);
  digitalWrite(L_EN_lift, HIGH);

  // for catching Squeeze motor lagori
  pinMode(IN1_LIFT_LAGO, OUTPUT); // no for PWM , direct 5V
  pinMode(IN2_LIFT_LAGO, OUTPUT);

  // for catching Squeeze motor lagori
  pinMode(IN1_SQUEEZE_LAGO, OUTPUT);
  pinMode(IN2_SQUEEZE_LAGO, OUTPUT);
  //run_speed(1, 0);

  //initilize servo
  initialPosition();
  
  initialRunningMotor();

}

void loop() {
  if (error == 1)
    return;
  else { //DualShock Controller
    ps2x.read_gamepad(false, vibrate);          //read controller and set large motor to spin at 'vibrate' speed
    if (ps2x.Button(PSB_START))                  //will be TRUE as long as button is pressed
      Serial.println("Start is being held");
    if (ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");
    if (ps2x.Button(PSB_PAD_UP)) {        //will be TRUE as long as button is pressed
      liftDir = ps2x.Analog(PSAB_PAD_UP);
    }
    if (ps2x.Button(PSB_PAD_RIGHT)) {
      Serial.print("Right held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_RIGHT), DEC);
      rollDir = ps2x.Analog(PSAB_PAD_RIGHT);
    }
    if (ps2x.Button(PSB_PAD_LEFT)) {
      Serial.print("LEFT held this hard: ");
      Serial.println(ps2x.Analog(PSAB_PAD_LEFT), DEC);
      rollDir = map(ps2x.Analog(PSAB_PAD_RIGHT), 0, 255, -255, 0);
    }
    if (ps2x.Button(PSB_PAD_DOWN)) {
      //  Serial.print("DOWN held this hard: ");
      //  Serial.println(ps2x.Analog(PSAB_PAD_DOWN), DEC);
      liftDir = map(ps2x.Analog(PSAB_PAD_UP), 0, 255, -255, 0);
    }
    //  vibrate = ps2x.Analog(PSAB_BLUE);
    vibrate = 0;
    if (ps2x.NewButtonState())
    {
      if (ps2x.Button(PSB_L3))
        Serial.println("L3 pressed");
      if (ps2x.Button(PSB_R3))
        Serial.println("R3 pressed");
      if (ps2x.Button(PSB_L2)) {
        Serial.println("L2 pressed");
        if (squeeze_state != SQUEEZE_UP) {
          Serial.println("Squeeze ball up");
          for (int i = 180; i >= 0; i--) {
            control_R1(i);
            control_M1(i);
            control_L1(i);
            delay(1);
          }
          squeeze_state = SQUEEZE_UP;
        } else {
          Serial.println("Squeeze ball down");
          for (int i = 0; i <= 180; i++) {
            control_R1(i);
            control_M1(i);
            control_L1(i);
            delay(1);
          }
          squeeze_state = SQUEEZE_DOWN;
        }
      }
      if (ps2x.Button(PSB_R2)) {
        Serial.println("R2 pressed");
        if (catchState != SQUEEZE_BALL) {

          Serial.println("Squeeze ball");
          for (int i = 0; i <= 180; i++) {
            control_R31(i);
            control_R32(i);
            control_M31(i);
            control_M32(i);
            control_L31(i);
            control_L32(i);
            delay(1);
          }
          catchState = SQUEEZE_BALL;
        } else {
          Serial.println("Roll ball");
          for (int i = 180; i >= 0; i--) {
            control_R31(i);
            control_R32(i);
            control_M31(i);
            control_M32(i);
            control_L31(i);
            control_L32(i);
            delay(1);
          }
          catchState = ROLL_BALL;
        }
      }
      if (ps2x.Button(PSB_GREEN))
        Serial.println("Triangle pressed");
    }
    if (ps2x.ButtonPressed(PSB_RED)) {           //will be TRUE if button was JUST pressed
      //Define rotate squeeze ball
      //      #define SQUEEZE_ROTATE_T 1
      //      #define SQUEEZE_ROTATE_F 0
      //
      //      //initialize alternate rotate squeeze state
      //      int squeeze_rotate_state = SQUEEZE_ROTATE_T;
      Serial.println("Circle just pressed");
      if (squeeze_rotate_state != SQUEEZE_ROTATE_T) {
        for (int i = 0; i <= 90; i++) {
          control_R2(i);
          control_M2(i);
          control_L2(i);
        }
        squeeze_rotate_state = SQUEEZE_ROTATE_T;
      } else {
        for (int i = 90; i >= 0; i--) {
          control_R2(i);
          control_M2(i);
          control_L2(i);
        }
        squeeze_rotate_state = SQUEEZE_ROTATE_F;
      }
    }
    if (ps2x.ButtonReleased(PSB_PINK))            //will be TRUE if button was JUST released
      Serial.println("Square just released");
    if (ps2x.NewButtonState(PSB_BLUE)) {          //will be TRUE if button was JUST pressed OR released
      Serial.println("X just changed");
    }

    /*==for running motor input read====*/
    js_left_x = ps2x.Analog(PSS_LX);
    js_left_y = ps2x.Analog(PSS_LY);
    js_right_x = ps2x.Analog(PSS_RX);
    js_right_y = ps2x.Analog(PSS_RY);
    //
    if (ps2x.Button(PSB_L1)) {
      additional_speed = SLOWER;
      Serial.println("L1");
    }
    if (ps2x.Button(PSB_R1)) {
      additional_speed = FASTER;
      Serial.println("L1");
    }
    if (ps2x.Button(PSB_RED)) {
      Serial.println("Circle is pressed");
    }
    if (ps2x.Button(PSB_PINK)) {
      Serial.println("Square is pressed");
    }
    if (ps2x.Button(PSB_BLUE)) {
      Serial.println("X is pressed");
      lift_catch_dir = -1;
      lift_speed = lift_speed_default_slow;
    }
    if (ps2x.Button(PSB_GREEN)) {
      Serial.println("Triangle is pressed");
      lift_catch_dir = 1;
      lift_speed = lift_speed_default;
    }
    if (ps2x.ButtonPressed(PSB_SELECT)) {
      mode_speed++;
    }
  }
  if (mode_speed == 3) {
    mode_speed = 1;
  }
  /*=================For streching motor======================*/
  setLift(lift_catch_dir, lift_speed, R_PWM_lift, L_PWM_lift); // (direction, speed, right_pwm_pin, left_pwm_pin)
  if (rollDir == 0) {
    catch_state = 0;
    squeeze_lago_speed = 0;
  } else if (rollDir > 0) {
    catch_state = 1; //in
    squeeze_lago_speed = SQUEEZE_DEFAULT_SPEED;
  } else if (rollDir < 0) {
    catch_state = -1; // out
    squeeze_lago_speed = SQUEEZE_DEFAULT_SPEED;
  }

  /*==============Exercute catch logo=========================*/
  setSqueezeMotor(catch_state, squeeze_lago_speed, PWM_SQUEEZE_LAGO, IN1_SQUEEZE_LAGO, IN2_SQUEEZE_LAGO);

  /*=================For lift motor lagory======================*/
  if (liftDir == 0) {
    stateDir = 0;
  } else if (liftDir > 0) {
    stateDir = GO_UP;
  } else if (liftDir < 0) {
    stateDir = GO_DOWN;
  }
  //  #define IN1_LIFT_LAGO 50
  //#define IN2_LIFT_LAGO 51
  //#define PWM_LIFT_LAGO 49
  setLiftLagori(stateDir, PWM_LIFT_LAGO, IN1_LIFT_LAGO, IN2_LIFT_LAGO);

  //Giving direction codition to motor
  //GIve to Y axis of Left and right JOYStick

  //  if(js_left_y <127)

  /*=================calculate the speed======================*/
  if (mode_speed == 1) {
    velo = DEFAULT_SPEED + additional_speed;
  } else if (mode_speed == 2) {
    velo = SPEED_RUN_LAGO;
  }


  Serial.print(mode_speed);
  Serial.print(",");
  Serial.println(velo);


  /*=================For running motor======================*/
  /*=================Left Joystick===============*/
  if(js_left_y == 0){
    setRunningMotorDriftLeft(velo);
  }
  else if(js_left_y == 255){
    setRunningMotorDriftRight(velo);
  }
  else if(js_left_x == 0){
    setRunningMotorLeftward(velo);
  }
  else if(js_left_x == 255){
    setRunningMotorRightward(velo);
  }
  /*=================Right Joystick===============*/
  else if(js_right_y == 0){
    setRunningMotorToward(velo);
  }
  else if(js_right_y == 255){
    setRunningMotorBackward(velo);
  }
  else if(js_right_x == 0){
    setRunningMotorRotateLeft(velo);
  }
  else if(js_right_x == 255){
    setRunningMotorRotateRight(velo);
  }else{
    setRunningMotorStop();
  }
  /*=================Reset everthing part======================*/

  //resut the velo and additional speed
  additional_speed = 0;
  velo = 0;


  //reset js(Joystick) state
  js_left_x = 128;
  js_left_y = 127;
  js_right_x = 128;
  js_right_y = 127;



  //resetPart
  liftDir = 0;// reset to the orginal state
  stateDir = 0;

  rollDir = 0;
  // Serial.println(catch_state);
  catch_state = 0;
  squeeze_lago_speed = 0;

  /*==============Reset lift catch direction and speed=================*/
  lift_catch_dir = 0;
  lift_speed = 0;
  delay(40);
}
void setLift(int dir, int pwmVal, int R_PWM, int L_PWM) {
  if (dir == 1) {
    analogWrite(R_PWM, pwmVal);
    analogWrite(L_PWM, 0);
  } else if (dir == -1) {
    analogWrite(R_PWM, 0);
    analogWrite(L_PWM, pwmVal);
  } else {
    analogWrite(R_PWM, 0);
    analogWrite(L_PWM, 0);
  }
}
void setLiftLagori(int dir, int pwm, int in1, int in2) {
  analogWrite(pwm, 255);
  if (dir == 1) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else if (dir == -1) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void setSqueezeMotor(int dir, int pwmVal, int pwm, int in1, int in2) {
  analogWrite(pwm, pwmVal);
  if (dir == 1) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else if (dir == -1) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}
