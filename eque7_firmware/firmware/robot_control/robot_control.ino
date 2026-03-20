#include <PID_v1.h>

// ========================== MOTOR DRIVER (L298N) PINS ==========================
#define L298N_enA 9  // PWM right
#define L298N_enB 11  // PWM left

#define L298N_in1 12  // Dir Motor A
#define L298N_in2 13  // Dir Motor A
#define L298N_in3 7  // Dir Motor B
#define L298N_in4 8  // Dir Motor B

// ========================== ENCODER CONNECTION PINS ==========================
#define right_encoder_phaseA 3  // Interrupt green right_motor
#define right_encoder_phaseB 5  //yellow right_motor
#define left_encoder_phaseA 2   // Interrupt green left_motor
#define left_encoder_phaseB 4   //yellow left_motor

// ========================== ENCODER & VELOCITY VARIABLES ==========================
unsigned int right_encoder_counter = 0;
unsigned int left_encoder_counter = 0;
String right_wheel_sign = "p";  // 'p' = positive, 'n' = negative
String left_wheel_sign = "p";  // 'p' = positive, 'n' = negative
unsigned long last_millis = 0;
const unsigned long interval = 100; // 100 ms loop for velocity update
double gear_reduction_ratio = 35.0; // 35:1 gearbox
double tick_per_rotation = 11.0;  // 11 pulses per motor shaft revolution

// ========================== SERIAL COMMAND PARSING VARIABLES ==========================
// These help decode messages like: "rp08.45,lp08.45,"
bool is_right_wheel_cmd = false;
bool is_left_wheel_cmd = false;
bool is_right_wheel_forward = true;
bool is_left_wheel_forward = true;
char value[] = "00.00";
uint8_t value_idx = 0;
bool is_cmd_complete = false;

// ========================== PID CONTROL VARIABLES ==========================
// Setpoints (desired velocities in rad/s)
double right_wheel_cmd_vel = 0.0;
double left_wheel_cmd_vel = 0.0;
// Inputs (measured velocities in rad/s)
double right_wheel_meas_vel = 0.0;
double left_wheel_meas_vel = 0.0;
// Outputs (PWM command 0–255)
double right_wheel_cmd = 0.0;
double left_wheel_cmd = 0.0;
// PID Tuning
// original
// double Kp_r = 11.5;
// double Ki_r = 7.5;
// double Kd_r = 0.1;
// double Kp_l = 12.8;
// double Ki_l = 8.3;
// double Kd_l = 0.1;


// double Kp_r = 40.0;
// double Ki_r = 0.8;
// double Kd_r = 0.0;
// double Kp_l = 35.0;
// double Ki_l = 0.5;
// double Kd_l = 0.0;

double Kp_r = 21.0;
double Ki_r = 4.1;
double Kd_r = 0.0;
double Kp_l = 20.0;
double Ki_l = 4.0;
double Kd_l = 0.0;
// Controller
PID rightMotor(&right_wheel_meas_vel, &right_wheel_cmd, &right_wheel_cmd_vel, Kp_r, Ki_r, Kd_r, DIRECT);
PID leftMotor(&left_wheel_meas_vel, &left_wheel_cmd, &left_wheel_cmd_vel, Kp_l, Ki_l, Kd_l, DIRECT);

void setup() {
  // ========================== MOTOR DRIVER SETUP ==========================
  pinMode(L298N_enA, OUTPUT);
  pinMode(L298N_enB, OUTPUT);
  pinMode(L298N_in1, OUTPUT);
  pinMode(L298N_in2, OUTPUT);
  pinMode(L298N_in3, OUTPUT);
  pinMode(L298N_in4, OUTPUT);

  // Set Motor Rotation Direction
  digitalWrite(L298N_in1, HIGH);
  digitalWrite(L298N_in2, LOW);
  digitalWrite(L298N_in3, HIGH);
  digitalWrite(L298N_in4, LOW);

  // ========================== PID CONTROLLER SETUP ==========================
  rightMotor.SetMode(AUTOMATIC);
  leftMotor.SetMode(AUTOMATIC);

  // ========================== SERIAL & ENCODER INTERRUPTS ==========================
  Serial.begin(115200);
  pinMode(right_encoder_phaseB, INPUT);
  pinMode(left_encoder_phaseB, INPUT);
  // Set Callback for Wheel Encoders Pulse
  attachInterrupt(digitalPinToInterrupt(right_encoder_phaseA), rightEncoderCallback, RISING);
  attachInterrupt(digitalPinToInterrupt(left_encoder_phaseA), leftEncoderCallback, RISING);
}

void loop() {
  // ========================== 1. READ SERIAL COMMANDS ==========================
  // Expect messages like: rp08.45,lp08.45,
  if (Serial.available())
  {
    char chr = Serial.read();
    // Right Wheel Motor
    if(chr == 'r')
    {
      is_right_wheel_cmd = true;
      is_left_wheel_cmd = false;
      value_idx = 0;
      is_cmd_complete = false;
    }
    // Left Wheel Mo tor
    else if(chr == 'l')
    {
      is_right_wheel_cmd = false;
      is_left_wheel_cmd = true;
      value_idx = 0;
    }
    // Positive direction
    else if(chr == 'p')
    {
      if(is_right_wheel_cmd && !is_right_wheel_forward)
      {
        // change the direction of the rotation
        digitalWrite(L298N_in1, HIGH - digitalRead(L298N_in1));
        digitalWrite(L298N_in2, HIGH - digitalRead(L298N_in2));
        is_right_wheel_forward = true;
      }
      else if(is_left_wheel_cmd && !is_left_wheel_forward)
      {
        // change the direction of the rotation
        digitalWrite(L298N_in3, HIGH - digitalRead(L298N_in3));
        digitalWrite(L298N_in4, HIGH - digitalRead(L298N_in4));
        is_left_wheel_forward = true;
      }
    }
    // Negative direction
    else if(chr == 'n')
    {
      if(is_right_wheel_cmd && is_right_wheel_forward)
      {
        // change the direction of the rotation
        digitalWrite(L298N_in1, HIGH - digitalRead(L298N_in1));
        digitalWrite(L298N_in2, HIGH - digitalRead(L298N_in2));
        is_right_wheel_forward = false;
      }
      else if(is_left_wheel_cmd && is_left_wheel_forward)
      {
        // change the direction of the rotation
        digitalWrite(L298N_in3, HIGH - digitalRead(L298N_in3));
        digitalWrite(L298N_in4, HIGH - digitalRead(L298N_in4));
        is_left_wheel_forward = false;
      }
    }
    // Separator: When a comma is received, a full velocity number has been read
    else if(chr == ',')
    {
      if(is_right_wheel_cmd)
      {
        right_wheel_cmd_vel = atof(value);
      }
      else if(is_left_wheel_cmd)
      {
        left_wheel_cmd_vel = atof(value);
        is_cmd_complete = true;
      }
      // Reset for next command
      value_idx = 0;
      value[0] = '0';
      value[1] = '0';
      value[2] = '.';
      value[3] = '0';
      value[4] = '0';
      value[5] = '\0';
    }
    // Command Value
    else
    {
      if(value_idx < 5)
      {
        value[value_idx] = chr;
        value_idx++;
      }
    }
  }

  // ========================== 2. ENCODER MEASUREMENT (every 100ms) ==========================
  unsigned long current_millis = millis();
  if(current_millis - last_millis >= interval)
  {
    right_wheel_meas_vel = (10 * right_encoder_counter * (60.0/(tick_per_rotation * gear_reduction_ratio))) * 0.10472;
    left_wheel_meas_vel = (10 * left_encoder_counter * (60.0/(tick_per_rotation * gear_reduction_ratio))) * 0.10472;
    
    // ========================== 3. PID COMPUTATION ==========================
    rightMotor.Compute();
    leftMotor.Compute();

    // Stop the motor if command velocity is zero
    if(right_wheel_cmd_vel == 0.0)
    {
      right_wheel_cmd = 0.0;
    }
    if(left_wheel_cmd_vel == 0.0)
    {
      left_wheel_cmd = 0.0;
    }

    // ========================== 4. SEND FEEDBACK TO ROS ==========================
    // This message format will be read by the hardware_interface (eg. r08.45,l07.92,)
    String encoder_read = "r" + right_wheel_sign + String(right_wheel_meas_vel) + ",l" + left_wheel_sign + String(left_wheel_meas_vel) + ",";
    Serial.println(encoder_read);

    // Reset for next cycle
    last_millis = current_millis;
    right_encoder_counter = 0;
    left_encoder_counter = 0;

    // ========================== 5. DRIVE MOTORS USING PWM ==========================
    analogWrite(L298N_enA, right_wheel_cmd);
    analogWrite(L298N_enB, left_wheel_cmd);
  }
}

// New pulse from Right Wheel Encoder
void rightEncoderCallback()
{
  if(digitalRead(right_encoder_phaseB) == HIGH)
  {
    right_wheel_sign = "p";
  }
  else
  {
    right_wheel_sign = "n";
  }
  right_encoder_counter++;
}

// ========================== ENCODER INTERRUPT CALLBACKS ==========================
// Count one tick every rising edge on Phase A, and check Phase B to get direction
void leftEncoderCallback()
{
  if(digitalRead(left_encoder_phaseB) == HIGH)
  {
    left_wheel_sign = "n";
  }
  else
  {
    left_wheel_sign = "p";
  }
  left_encoder_counter++;
}