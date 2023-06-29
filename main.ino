#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <FaBoLCDmini_AQM0802A.h>

FaBoLCDmini_AQM0802A lcd; // Initialize LCD screen object

/*******************************/
/****** setting variables ******/
/*******************************/

// Define pin numbers for various controls and measurements
const int HV_pin=9; // Control pin for the high voltage source
const int CPC_pin=A1; // Read pin for the particle concentration sensor (CPC)
const int Flow_pin=A2; // Read pin for the sheath gas flow rate sensor
const int Temp_pin=A0; // Read pin for the temperature sensor

const int HV_state_pin=A3; // Read pin for the high voltage switch state
const int V_pin=2; // Control pin for voltage source

// Define variables for different measurements and parameters
const int NiV=10; // Number of voltage channels
int iV[NiV]; // Array of voltage index
float V[NiV]; // Array of voltage values
float temp; // Variable for temperature 
float val; // Variable for sensor reading
float C;   // Variable for concentration
float Qsh; // Variable for sheath gas flow rate
int sample_num=0; // Index for sample 
int HV_state; // Variable for high voltage switch state
int HV0;

File resultFile; // Object for the SD card file


/*******************************/
/****** setting functions ******/
/*******************************/

// Function to convert temperature sensor reading (voltage) to temperature
double compute_T(float x) {return -(1000 * (x * 5 / 1024.0) - 1546.0) / 8.2;} 

// Function to convert flow rate sensor reading (voltage) to flow rate
void compute_Q()
{
  Qsh=0;
  // Average 100 flow rate sensor readings
  for(int i=0; i<100; i++) {
    Qsh += analogRead(Flow_pin)/1024.0*0.05;
    delay(10);
  }
  // Convert sensor reading to flow rate
  Qsh=(Qsh-1.107)/0.919;
  // Print the flow rate and sample index to the LCD screen
  lcd.setCursor(0, 1);lcd.print("Q="); lcd.print(String(Qsh,1)); lcd.print(" "); lcd.print(sample_num);
}

void setup(){
  // Initialize the high voltage control pin and the high voltage source
  pinMode( V_pin, OUTPUT ); 
  digitalWrite(V_pin,HIGH); 
  HV0=analogRead(HV_state_pin);
  
  // Define the voltage values for the high voltage source
  iV[0]=28; iV[1]=29; iV[2]=30; iV[3]=32; iV[4]=36; iV[5]=40; iV[6]=48; iV[7]=60; iV[8]=82; iV[9]=116;
  V[0]=10; V[1]=17; V[2]=26; V[3]=44; V[4]=88; V[5]=136; V[6]=238; V[7]=400; V[8]=710; V[9]=1199;
  analogWrite(HV_pin,0); 

  // Initialize serial communication
  Serial.begin(9600); 
  
  // Initialize the LCD screen
  lcd.begin();
  lcd.print("Hi, TAMA!");
}



/*******************************/
/******     main loop     ******/
/*******************************/

void loop() {
  // If the high voltage switch is on, perform measurements and data recording
  if (analogRead(HV_state_pin)==1023){ 
    lcd.setCursor(0, 1);lcd.print("        ");lcd.setCursor(0, 0);lcd.print("        "); 
    temp = compute_T(analogRead(Temp_pin)); 
    compute_Q();
    
    // Cycle through different high voltage values
    for(int i=0; i<NiV; i++){
      // Set the high voltage value
      analogWrite(HV_pin,iV[i]); 
      lcd.setCursor(0, 0);lcd.print("V="); lcd.print(String(V[i],0));
      
      // Open the SD card file for writing
      resultFile = SD.open("V_C-2.txt", FILE_WRITE);
      C=0;
      delay(5000);

      // Measure the particle concentration 5 times
      for(int iloop=0; iloop<5; iloop++){ 
        val=analogRead(CPC_pin); 
        C+=pow(10,val/1023.0*5+2); 
        delay(1000);
      }
      
      // Write the average concentration and the voltage to the SD card file
      resultFile.print(V[i]); resultFile.print("\t"); resultFile.println(C/5.0);resultFile.close();
      
      // Read and display the flow rate
      compute_Q();
    }
    sample_num++;
    analogWrite(HV_pin,0); 
  }
  // If the high voltage switch is off, display a greeting message and measure the flow rate
  else {
    lcd.setCursor(0, 0);lcd.print("        ");
    lcd.print("Hi, TAMA!");
    lcd.setCursor(0, 1);lcd.print("        ");
    compute_Q();
    analogWrite(HV_pin,0);
    delay(5000);
  }
}
