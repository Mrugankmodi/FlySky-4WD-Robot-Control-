// ===============================
// MOTOR PINS
// ===============================

int enA = 5;
int in1 = 2;
int in2 = 3;

int enB = 6;
int in3 = 4;
int in4 = 7;


// ===============================
// RECEIVER PINS
// CH1 = STEERING
// CH2 = THROTTLE
// ===============================

int receiver_pins[] = {
  A0, A1, A2, A3, A4, A5
};

int receiver_values[] = {
  0, 0, 0, 0, 0, 0
};


// ===============================
// RECEIVER RANGE
// ===============================

int steer_min = 994;
int steer_max = 1968;
int steer_center = 1476;

int throttle_min = 1113;
int throttle_max = 1988;
int throttle_center = 1610;


// ===============================
// SETUP
// ===============================

void setup() {

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  Serial.begin(115200);
}


// ===============================
// MAIN LOOP
// ===============================

void loop() {

  receive();

  // CH1 = STEERING
  int steering = receiver_values[0];

  // CH2 = THROTTLE
  int throttle = receiver_values[1];


  // ===============================
  // DEAD ZONE
  // ===============================

  if (abs(throttle) < 15) {
    throttle = 0;
  }

  if (abs(steering) < 15) {
    steering = 0;
  }


  // =====================================================
  // SPECIAL FULL-SPEED TURN
  //
  // When throttle is high AND steering is high:
  //
  // FULL FORWARD + LEFT:
  // LEFT  = BACKWARD 255
  // RIGHT = FORWARD  255
  //
  // FULL FORWARD + RIGHT:
  // LEFT  = FORWARD  255
  // RIGHT = BACKWARD 255
  //
  // Both motors are always running.
  // =====================================================

  if (abs(throttle) >= 220 && abs(steering) >= 220) {

    // -------------------------------
    // FORWARD + LEFT
    // -------------------------------

    if (throttle > 0 && steering > 0) {

      mpower(1, -255);
      mpower(2, 255);
    }


    // -------------------------------
    // FORWARD + RIGHT
    // -------------------------------

    else if (throttle > 0 && steering < 0) {

      mpower(1, 255);
      mpower(2, -255);
    }


    // -------------------------------
    // BACKWARD + LEFT
    // -------------------------------

    else if (throttle < 0 && steering > 0) {

      mpower(1, 255);
      mpower(2, -255);
    }


    // -------------------------------
    // BACKWARD + RIGHT
    // -------------------------------

    else if (throttle < 0 && steering < 0) {

      mpower(1, -255);
      mpower(2, 255);
    }

    return;
  }


  // =====================================================
  // NORMAL DIFFERENTIAL DRIVE
  //
  // Used for normal turning and speed control.
  // =====================================================

  int leftMotor  = throttle + steering;
  int rightMotor = throttle - steering;


  // ===============================
  // NORMALIZE
  // ===============================

  int maximum =
    max(abs(leftMotor), abs(rightMotor));

  if (maximum > 255) {

    leftMotor =
      (leftMotor * 255L) / maximum;

    rightMotor =
      (rightMotor * 255L) / maximum;
  }


  // ===============================
  // MOTOR OUTPUT
  // ===============================

  mpower(1, leftMotor);
  mpower(2, rightMotor);
}


// ===============================
// RECEIVE RC SIGNAL
// ===============================

int rp = 0;

void receive() {

  unsigned long pulse =
    pulseIn(
      receiver_pins[rp],
      HIGH,
      30000
    );


  if (pulse > 0) {

    // ===========================
    // CH1 = STEERING
    //
    // LEFT   = 1968
    // CENTER = 1476
    // RIGHT  = 994
    // ===========================

    if (rp == 0) {

      if (pulse >= steer_center) {

        receiver_values[rp] =
          map(
            pulse,
            steer_center,
            steer_max,
            0,
            -255
          );

      } else {

        receiver_values[rp] =
          map(
            pulse,
            steer_min,
            steer_center,
            255,
            0
          );
      }
    }


    // ===========================
    // CH2 = THROTTLE
    //
    // UP      = 1988
    // CENTER  = 1610
    // DOWN    = 1113
    //
    // REVERSED:
    //
    // DOWN = FORWARD
    // UP   = BACKWARD
    // ===========================

    else if (rp == 1) {

      if (pulse >= throttle_center) {

        // UP = BACKWARD

        receiver_values[rp] =
          map(
            pulse,
            throttle_center,
            throttle_max,
            0,
            -255
          );

      } else {

        // DOWN = FORWARD

        receiver_values[rp] =
          map(
            pulse,
            throttle_min,
            throttle_center,
            255,
            0
          );
      }
    }


    // ===========================
    // OTHER CHANNELS
    // ===========================

    else {

      receiver_values[rp] = pulse;
    }


    // ===========================
    // LIMIT CH1 AND CH2
    // ===========================

    if (rp == 0 || rp == 1) {

      receiver_values[rp] =
        constrain(
          receiver_values[rp],
          -255,
          255
        );
    }
  }


  rp++;

  if (rp >= 6) {
    rp = 0;
  }
}


// ===============================
// MOTOR POWER
// ===============================

void mpower(int motor, int spd) {

  int rotation = 0;


  // ===============================
  // FORWARD
  // ===============================

  if (spd > 0) {

    rotation = 1;
  }


  // ===============================
  // BACKWARD
  // ===============================

  else if (spd < 0) {

    rotation = -1;

    spd = -spd;
  }


  // ===============================
  // PWM LIMIT
  // ===============================

  if (spd > 255) {
    spd = 255;
  }


  int pwm;
  int pA;
  int pB;


  // ===============================
  // LEFT MOTOR
  // ===============================

  if (motor == 1) {

    pwm = enA;
    pA = in1;
    pB = in2;
  }


  // ===============================
  // RIGHT MOTOR
  // ===============================

  else if (motor == 2) {

    pwm = enB;
    pA = in3;
    pB = in4;
  }

  else {
    return;
  }


  // ===============================
  // STOP
  // ===============================

  if (rotation == 0) {

    digitalWrite(pA, LOW);
    digitalWrite(pB, LOW);

    analogWrite(pwm, 0);
  }


  // ===============================
  // FORWARD
  // ===============================

  else if (rotation == 1) {

    digitalWrite(pA, HIGH);
    digitalWrite(pB, LOW);

    analogWrite(pwm, spd);
  }


  // ===============================
  // BACKWARD
  // ===============================

  else {

    digitalWrite(pA, LOW);
    digitalWrite(pB, HIGH);

    analogWrite(pwm, spd);
  }
}