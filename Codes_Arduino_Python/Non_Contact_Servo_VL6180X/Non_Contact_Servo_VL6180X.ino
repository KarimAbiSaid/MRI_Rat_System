
//The range readings are in units of mm. */

// parameters: Gain of motion, filtering amount, force on rat, time of the protocol
// measurements: amplitude of movement: how far the rat can push - to calibrate motion
// measurements: amplitude of rat's reaction on touch
// Plots: Rat's arm movement, in sync with the motion of the motor, in sync with the contact, and finally the activity of the rat.

#include <Wire.h>
#include <VL6180X.h>

#include <Servo.h>

// Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// Define the size of the circular buffer
const int bufferSize = 200;

int sensitivity_contact = 400; //between 0 and 1023:      0 --> most sensitive,       1023 --> least sensitive.

// Define the delay in milliseconds
int delay_time = -1;
int min_servo_position = 0;
int max_servo_position = 179;


int angle;              // variable to hold the angle for the servo motor
int counter = 0;

int avg_position;
int previous_avg_position;

int current_position;
int previous_current_position;

int saved_counter;
int max_angle = 180;



//
const int potPin = A1;


// Define your servo pin
const int servoPin = 9;

//delay of the loop
int loop_delay = 1;
const int timing_budget = 200;

// Define your position sensor pin
const int sensorPin = A0;



// Create a circular buffer to store position history
int positionBuffer[bufferSize];
int bufferIndex = 0;

// Create a Servo object
Servo myServo;
Servo myServo2;

int potVal;


VL6180X sensor;



void setup()
{
  Serial.begin(115200);
  Wire.begin();

  
  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);


  saved_counter = 0;
  


  // Attach the servo to the servo pin
  myServo.attach(servoPin);
  // Serial.println("hello");

}









static bool is_calibrated = false;
static bool min_max_found = false;
int indices_backwards = 0;
int number_of_loops = 0;
int loop_time = 0;

int distance;
int previousPositionIndex;
int previousPosition;

int previousAvgPositionIndex;
int temp_previous = 179;
int contact_value;

int min_dist = 1000;
int max_dist = 0;


int calibration_min_max_time = millis();

void Calibration_min_max(){
  

  Serial.println("NOW CALIBRATING MINIMUM, MOVE TO FURTHEST DISTANCE INSIDE");
  while ((millis() - calibration_min_max_time) < 4000){
      distance = sensor.readRangeSingleMillimeters();
      if (distance < min_dist){
        min_dist = distance;
      }
  }
  calibration_min_max_time = millis();
  
  Serial.println("NOW CALIBRATING MAXIMUM, MOVE TO FURTHEST DISTANCE OUTSIDE");
  while ((millis() - calibration_min_max_time) < 4000){
      distance = sensor.readRangeSingleMillimeters();
      if (distance > max_dist){
        max_dist = distance;
      }
  }

  min_max_found = true;


}


void SerialReadLoop(){

  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n'); // Read the incoming string until newline character
    delay_time = command.toInt(); // Convert the string to an integer
  }
  return;
}



int calibration_time = millis();
int calibration_extra_time = 0;

void Calibration_loop() {


  distance = sensor.readRangeSingleMillimeters();
    // Serial.println(distance);
  distance = (round(distance));
  if (distance <= min_dist){distance=min_dist;}
  if (distance >=max_dist){
    distance = max_dist;
  }

  // potVal = analogRead(potPin);

  
  current_position = map(distance,min_dist,max_dist,min_servo_position,max_servo_position);
  
  // myServo.write(previousPosition);
  contact_value = analogRead(A0);
  if (contact_value > sensitivity_contact){
    // Serial.println("TOUCIHING");
  // if (0){
    //if contact

    if (current_position - positionBuffer[(bufferIndex + bufferSize - 1)%bufferSize] >= 0){ // if still moving forward
      // myServo.write(map(last_saved_previous_position,15,-15,0,180));
      // Update the circular buffer with the current position


      
      current_position = positionBuffer[(bufferIndex + bufferSize - 1)%bufferSize];
      previousPositionIndex = (bufferIndex + bufferSize - delay_time/(timing_budget))% bufferSize;
      previousPosition = positionBuffer[previousPositionIndex];
      bufferIndex = (bufferIndex + 1) % bufferSize;
      avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;

      max_angle = avg_position;
      return;


      // Write the previous position to the servo
      

    }

   }

  else if (current_position < max_angle){

    positionBuffer[bufferIndex] = current_position; // update the current buffer index
    // Get the position from "delay_time" ago
    // int previousPositionIndex = (bufferIndex + bufferSize - (delay_time / loop_delay)) % bufferSize;
    previousPositionIndex = (bufferIndex + bufferSize - delay_time/(timing_budget))% bufferSize;
    previousPosition = positionBuffer[previousPositionIndex];

    avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize])/4;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize])/3;
    bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
    // Serial.println(previousPosition);
    if (avg_position <max_angle){
    myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position));}

   

    
  }
  else if (current_position >= max_angle){
    positionBuffer[bufferIndex] = max_angle;
    // current_position = positionBuffer[(bufferIndex + bufferSize - 1)%bufferSize];
    previousPositionIndex = (bufferIndex + bufferSize -1 - delay_time/(timing_budget))% bufferSize;
    previousPosition = positionBuffer[previousPositionIndex];
    avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize])/4;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize])/3;
    bufferIndex = (bufferIndex + 1) % bufferSize;
    // Serial.println(previousPosition);
    if (avg_position <max_angle){
    myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position));}

  }


  // Add a small delay to prevent rapid iteration

  // loop_delay = (millis()-time);
  // delay(loop_delay);
  number_of_loops = number_of_loops + 1;

  if (number_of_loops < 10){
    calibration_extra_time = millis()-calibration_time;
    return;
  }
  if (millis() - calibration_time > 2000){
    is_calibrated = true;
    loop_time = (millis() - calibration_time- calibration_extra_time)/(number_of_loops-10);
    if (loop_time<1){loop_delay = 4;}
    else{loop_delay = loop_time;}

    // Serial.println("Loop Start");
    if (delay_time > loop_delay){
      delay_time = delay_time - loop_delay;
      

    }
    
    return;

  }
  
}







void Running_loop() {
  int time = millis();
  // Serial.print("MaxAngle:");
  // Serial.println(max_angle);

  // Serial.print("Contact:");
  // Serial.println(analogRead(A0));

  // Serial.print("AveragePosition:");
  // Serial.println(avg_position);

  // Serial.print("PrevAvgPosition:");
  // Serial.println(previous_avg_position);

  // Serial.print("CurrentPosition:");
  // Serial.println(current_position);



  Serial.print(((millis()/(60)) /(60)) /1000); //hours
  Serial.print(":");
  Serial.print(((millis()/(60))/1000) % 60); //mins
  Serial.print(":");
  Serial.print((millis()/1000) % 60); //seconds
  Serial.print(":");
  Serial.print(millis()%1000); //millis
  Serial.print(',');



  Serial.print(max_angle);  
  Serial.print(',');


  Serial.print(delay_time + loop_delay);
  Serial.print(',');


  Serial.print(avg_position);
  Serial.print(',');

  Serial.print(contact_value);
  Serial.print(',');

  Serial.print(distance);

  
  Serial.println(',');

  distance = sensor.readRangeSingleMillimeters();
    // Serial.println(distance);
  distance = (round(distance));
  if (distance <= min_dist){distance=min_dist;}
  if (distance >=max_dist){
    distance = max_dist;
  }

  // potVal = analogRead(potPin);


  current_position = map(distance,min_dist,max_dist,min_servo_position,max_servo_position);

  previousAvgPositionIndex = (bufferIndex + bufferSize -2) % bufferSize; //since we already incremented it in the previous loop, doing -1 only would give us the previous avg position, and not the prev_avg_position
  previousAvgPositionIndex = (previousAvgPositionIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
  previous_avg_position = positionBuffer[previousAvgPositionIndex];

  previous_avg_position = (previous_avg_position + 0.75*positionBuffer[(previousAvgPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousAvgPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousAvgPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousAvgPositionIndex + bufferSize - 4)%bufferSize])/3;
  

  // myServo.write(previousPosition);
  contact_value = analogRead(A0);
  if (contact_value > sensitivity_contact){ // if we are touching the rat


    if((current_position >= max_angle)&&(avg_position >= previous_avg_position)){ //if are still forwards, but are still touching and the average position (the previous_position) is still touching
      // Update the circular buffer with the current position
      if (previous_avg_position <=max_angle){
      max_angle = previous_avg_position;}
      positionBuffer[bufferIndex] = max_angle; // update the current buffer index to the CURRENT LOCATION WHICH IS LESS THAN THE MAXIMUM ANGLE
      
      previousPositionIndex = (bufferIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
      previousPosition = positionBuffer[previousPositionIndex];

      avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
      
      bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
      myServo.write(map(max_angle,max_servo_position,min_servo_position,min_servo_position,max_servo_position));
      return;
    }

    else if((current_position <= max_angle) && (avg_position >= previous_avg_position)){ //if are still forwards, but are still touching and the average position (the previous_position) is still touching
      // Update the circular buffer with the current position
      if (previous_avg_position <=max_angle){
      max_angle = previous_avg_position;}
      positionBuffer[bufferIndex] = current_position; // update the current buffer index to the CURRENT LOCATION WHICH IS LESS THAN THE MAXIMUM ANGLE
      
      previousPositionIndex = (bufferIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
      previousPosition = positionBuffer[previousPositionIndex];

      avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
      
      bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
      myServo.write(map(max_angle,max_servo_position,min_servo_position,min_servo_position,max_servo_position));
      return;
    }



    else if ((avg_position <= max_angle) && (avg_position <= previous_avg_position)){ //if we are touching but moving backwards
      // Update the circular buffer with the current position

      positionBuffer[bufferIndex] = current_position; // update the current buffer index to the CURRENT LOCATION WHICH IS LESS THAN THE MAXIMUM ANGLE

      previousPositionIndex = (bufferIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
      previousPosition = positionBuffer[previousPositionIndex];

      avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
      bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
      
      myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position)); // update the servo position    
      
      return;

    }

    else if ((avg_position > max_angle) && (avg_position <= previous_avg_position)){ //if we are touching, but the previous position is still backwards, but the future position will cause it to touch
      // Update the circular buffer with the current position
      if (previous_avg_position <=max_angle){
      max_angle = previous_avg_position;}

      if (current_position >= max_angle){
        positionBuffer[bufferIndex] = max_angle; // update the current buffer index to the CURRENT LOCATION WHICH IS LESS THAN THE MAXIMUM ANGLE
      }
      else{positionBuffer[bufferIndex] = current_position;}
      previousPositionIndex = (bufferIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
      previousPosition = positionBuffer[previousPositionIndex];

      avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
      bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
      if (avg_position < max_angle){
      myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position)); // update the servo position  
      }  

      return;

    }

    

   }

  else if (current_position <= max_angle){ //not touching

    positionBuffer[bufferIndex] = current_position; // update the current buffer index
    // Get the position from "delay_time" ago
    // int previousPositionIndex = (bufferIndex + bufferSize - (delay_time / loop_delay)) % bufferSize;
    previousPositionIndex = (bufferIndex + bufferSize - delay_time/(loop_delay))% bufferSize;
    previousPosition = positionBuffer[previousPositionIndex];
    avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize])/4;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize])/3;
    bufferIndex = (bufferIndex + 1) % bufferSize; // Move to the next index, wrapping around if necessary
    // Serial.println(previousPosition);
    if (avg_position<max_angle){
      myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position));
    }


    
  }
  else if (current_position >= max_angle){
    
    positionBuffer[bufferIndex] = max_angle;
    // current_position = positionBuffer[(bufferIndex + bufferSize - 1)%bufferSize];
    previousPositionIndex = (bufferIndex + bufferSize -1 - delay_time/(loop_delay))% bufferSize;
    previousPosition = positionBuffer[previousPositionIndex];
    avg_position = (previousPosition + 0.75*positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + 0.5*positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize] + 0.25*positionBuffer[(previousPositionIndex + bufferSize - 4)%bufferSize])/3;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 3)%bufferSize])/4;
    // avg_position = (previousPosition + positionBuffer[(previousPositionIndex + bufferSize - 1)%bufferSize] + positionBuffer[(previousPositionIndex + bufferSize - 2)%bufferSize])/3;
    bufferIndex = (bufferIndex + 1) % bufferSize;
    
    // Serial.println(previousPosition);
    
    if (avg_position<max_angle){
      myServo.write(map(avg_position,max_servo_position,min_servo_position,min_servo_position,max_servo_position));
    }

  }


  // Add a small delay to prevent rapid iteration

  // loop_delay = (millis()-time);
  // delay(loop_delay);
  
}






void loop() {
  // if (!min_max_found){
  //   Calibration_min_max();
  // }
  min_dist = 20;
  max_dist = 51;
  if (delay_time < 0){
    SerialReadLoop();
  }
  if (!is_calibrated) {
    // Run calibration loop
    Calibration_loop();
  } 
  else {
    // Run main loop
    Running_loop();
   }
}


