/*
 *  ------Geiger Tube board (Arduino Code) Example--------
 *
 *  Explanation: This example shows how to get the signal from the Geiger Tube
 *  in Arduino, we use one of the Arduino interrupt pins (PIN2).
 *  We count the time (ms) between two pulses of the Geiger tube.
 *
 *  Copyright (C) 2011 Libelium Comunicaciones Distribuidas S.L.
 *  http://www.libelium.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Version:		0.3
 *  Design:		Marcos Yarza, David Gascon
 *  Implementation:	Marcos Yarza
 * 
 *  Version:    0.4
 *  Description: added cumulative counter over a period of five minutes
 *  Modified by: circulosmeos (circulosmeos.wordpress.com)
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(3,4,5,6,7,8);


// Threshold values for the led bar
#define TH1 45
#define TH2 95
#define TH3 200
#define TH4 400
#define TH5 600

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812

// 5 min of counts
#define BUFFER_COUNTS 30

// Variables
int ledArray [] = {10,11,12,13,9};
int geiger_input = 2;
long count = 0;
long countPerMinute = 0;
long timePrevious = 0;
long timePreviousMeassure = 0;
long time = 0;
long countPrevious = 0;
float radiationValue = 0.0;

// keeps the sum of counts for the ring
unsigned long cpm = 0;
// stores the pulses for a set of 30 interval (10 s) periods
unsigned long ring[BUFFER_COUNTS] = {0};
// pointer to the next cell in the ring to update
byte pointer = 0;
boolean buffer_full = false;
long countPerMinuteAccu = 0;
float radiationValueAccu = 0.0;

void setup(){
  pinMode(geiger_input, INPUT);
  digitalWrite(geiger_input,HIGH);
  for (int i=0;i<5;i++){
    pinMode(ledArray[i],OUTPUT);
  }

  Serial.begin(19200);

  //set up the LCD\'s number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Radiation Sensor");
  lcd.setCursor(0,1);
  lcd.print("Board - Arduino");  
  delay(1000);

  /*lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Cooking Hacks");
  delay(1000);*/
  /*
  lcd.clear();
  lcd.setCursor(0,1);  
  lcd.print("wait at least 5 min");
  delay(500);
  for (int i=0;i<5;i++){
    delay(200);  
    lcd.scrollDisplayLeft();
  }
  delay(500);
  */
  /*lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  - Libelium -");
  lcd.setCursor(0,1);
  lcd.print("www.libelium.com");    
  delay(1000);*/

  lcd.clear();  
  lcd.setCursor(0, 0);
  lcd.print("CPM=");
  lcd.setCursor(4,0);
  lcd.print(6*count);
  lcd.setCursor(0,1);
  lcd.print(radiationValue);

  attachInterrupt(0,countPulse,FALLING);

}

void loop(){
  if (millis()-timePreviousMeassure > 10000){

    countPerMinute = 6*count;
    radiationValue = countPerMinute * CONV_FACTOR;
    timePreviousMeassure = millis();
  
    // buffer calculus:
    // -----------------
    // calculate the moving sum of counts
    cpm = cpm - ring[pointer] + count;
    // store the current period value
    ring[pointer] = count;
    // move the pointer to the next position in the ring
    pointer = (pointer + 1) % BUFFER_COUNTS;
    if ( pointer == BUFFER_COUNTS-1 ) {
      buffer_full=true;
    }
        
    if (buffer_full == true) {
      countPerMinuteAccu = cpm/5;
    } else {
      countPerMinuteAccu = cpm*6/pointer;
    }
    radiationValueAccu = countPerMinuteAccu * CONV_FACTOR;
    // -----------------
    
    Serial.print("cpm = "); 
    Serial.print(countPerMinute,DEC);
    Serial.print(" - ");
    Serial.print("uSv/h = ");
    Serial.println(radiationValue,4);  

    Serial.print("Accu cpm = "); 
    Serial.print(countPerMinuteAccu,DEC);
    /*Serial.print("internal = "); 
    Serial.print(cpm,DEC);
    Serial.print(" - ");
    Serial.print(pointer,DEC);*/
    Serial.print(" - ");
    Serial.print("Accu uSv/h = ");
    Serial.println(radiationValueAccu,4);  

    lcd.clear();    
    lcd.setCursor(0, 0);
    lcd.print("CPM=");
    lcd.setCursor(4,0);
    lcd.print(countPerMinute);

    lcd.setCursor(8,0);
    lcd.print("Acc=");
    lcd.setCursor(12,0);
    lcd.print(countPerMinuteAccu);
    
    lcd.setCursor(0,1);
    lcd.print(radiationValue,3);
    lcd.setCursor(5,1);
    lcd.print("uSv/h");

    if (buffer_full == true) { // marks 5 minutes time completion
      lcd.setCursor(10,1);
      lcd.print("|");
    }
    
    lcd.setCursor(11,1);
    if ( buffer_full == true || 
      (pointer>15 && pointer%2==0) ) { // between 2.5 and 5 minutes, show both uSv/h and time elapsed
      lcd.print(radiationValueAccu,3);
    } else {
      lcd.print( pointer/6, DEC ); 
      lcd.setCursor(12,1);
      lcd.print( "m" );
      lcd.setCursor(13,1);
      if (pointer%6==0) {
        lcd.print("00");
      } else {
        lcd.print( (pointer%6)*10, DEC );        
      }
      lcd.setCursor(15,1);
      lcd.print( "s" );
    }

    //led var setting  
    if(countPerMinute <= TH1) ledVar(0);
    if((countPerMinute <= TH2)&&(countPerMinute>TH1)) ledVar(1);
    if((countPerMinute <= TH3)&&(countPerMinute>TH2)) ledVar(2);
    if((countPerMinute <= TH4)&&(countPerMinute>TH3)) ledVar(3);
    if((countPerMinute <= TH5)&&(countPerMinute>TH4)) ledVar(4);
    if(countPerMinute>TH5) ledVar(5);

    count = 0;

  }

}

void countPulse(){
  detachInterrupt(0);
  count++;
  while(digitalRead(2)==0){
  }
  attachInterrupt(0,countPulse,FALLING);
}

void ledVar(int value){
  if (value > 0){
    for(int i=0;i<=value;i++){
      digitalWrite(ledArray[i],HIGH);
    }
    for(int i=5;i>value;i--){
      digitalWrite(ledArray[i],LOW);
    }
  }
  else {
    for(int i=5;i>=0;i--){
      digitalWrite(ledArray[i],LOW);
    }
  }
}
