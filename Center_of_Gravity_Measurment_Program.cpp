// Do not remove the include below
#include "Center_of_Gravity_Measurment_Program.h"
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//add interrupt for measuring torque-

#define AREAD_PIN 0
#define SENSOR_2_ROTAXIS .100 // meters
#define SENSOR_RADIUS .3 // meters
#define GEOCENTER_2_PIVOT .06 //meters
//These declarations are sample analog read values measured during
//the calibration of the torque sensor
//these values would be changed to the actual values read during
//calibration
//Two known torques must be used to define a linear relationship
//Torque sensor output is assumed to scale linearly
float aRead1 = 100;
float aTorque1 = 40;
float aRead2 = 150;
float aTorque2 = 60;
//This value will be defined during the force taring protocol
float torque_nosat = 0;


long time = 0;
int interval = 1000;
int num_samples = 0;
float avg_torque = 0;
float sum_torques = 0;

int test_num = 1;
int dist_x = 0;
int dist_y = 0;
int dist_z = 0;
//Satellite weight must previously be measured
int sat_weight = 3;

boolean finished_measuring = false;
boolean Retest = false;
boolean torque_tared = false;

int ipin = 2;
volatile int ready = 0;

//ISR
ISR(INTO_vect)
{
	ready = digitalRead(ipin);
}

void setup()
{
  //Instead of printing to serial, a LCD screen may be used to display results
  //However print commands will differ for different models of LCD screens different LCD screen
  //support libraries
  Serial.begin(9600); //set up serial
  pinMode(ipin, INPUT); //Interrupt input pin
  GICR |= (1 << INTO); //enable global interrupt. This is only works for ATMega8 microcontroller.
  //Interrupt triggered by signal change
  MCUCR |=(1 << ISC00);
  MCUCR |=(0 << ISC01);

}

void measure_torques(int torque)
{
	  if (millis() > (time + interval) && num_samples < 5){
		  Serial.print("Reading ");
		  Serial.print(num_samples);
		  Serial.println();
		  sum_torques += torque;
		  num_samples++;
	  }

	  if(num_samples == 5){
		  avg_torque = sum_torques/num_samples;
		  num_samples = 0;
		  sum_torques = 0;
		  torque_tared = false;
		  finished_measuring = true;
	  }

}

// The loop function is called in an endless loop
void loop()
{

  if(num_samples = 0 && torque_tared == false){
	  if (ready){
		  float tare_reading = analogRead(AREAD_PIN);
		  torque_nosat = ((aTorque2 - aTorque1)/(aRead2 - aRead1))*(tare_reading - aRead1) + aTorque1;
		  torque_tared = true;
	  }
  }

  else if(num_samples = 0 && torque_tared == true){
	  while(!ready){
		  //waits for the operator to position the satellite and signal to the program that he or she is ready
	  }
  }
  else{

  float sensorReading = analogRead(AREAD_PIN);

  //use point-slope form to determine load reading based upon linear
  //relationship defined from calibration torque values

  float torque = ((aTorque2 - aTorque1)/(aRead2 - aRead1))*(sensorReading - aRead1) + aTorque1 - torque_nosat;

  measure_torques(torque);

  //torque sample readings and average torque calculated

  if (finished_measuring && test_num == 1){
	  dist_x = (avg_torque/ SENSOR_RADIUS) * SENSOR_2_ROTAXIS / sat_weight;
	  Serial.print("X distance is ");
	  Serial.print(dist_x);
	  Serial.println();
	  time = 0;
	  finished_measuring = false;
	  test_num++;
  }

  if (finished_measuring && test_num == 2){
	  dist_y = (avg_torque/ SENSOR_RADIUS) * SENSOR_2_ROTAXIS / sat_weight;
	  Serial.print("Y distance is ");
	  Serial.print(dist_y);
	  Serial.println();
	  time = 0;
	  finished_measuring = false;
	  test_num++;
  }

  if (finished_measuring && test_num == 3){
	  if(dist_x ==0){
		  Serial.print("X distance must be measured first!");
	  }
	  else{
	    float z2 = (avg_torque/ SENSOR_RADIUS) * SENSOR_2_ROTAXIS / sat_weight;
	    float z1 = dist_x;
	    float b = (z1 - z2) / tan(30 * PI / 180);
	    float dist_zt = b - z1*sin(30 * PI / 180);
	    dist_z = dist_zt + GEOCENTER_2_PIVOT * sin(30 * PI / 180);
		Serial.print("Z distance is ");
		Serial.print(dist_z);
		Serial.println();
	    time = 0;
	    finished_measuring = false;
	    test_num++;
      }
  if(test_num > 3){
	  if(Retest){
		 dist_x = 0;
		 dist_y = 0;
		 dist_z = 0;
		 test_num = 1;
	  }
	  else{
		  delay(1);
	  }
    }
  }
  }
}


