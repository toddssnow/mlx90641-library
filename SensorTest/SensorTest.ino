/*
    Read the temperature pixels from the MLX90641 IR array
*/

#include <Wire.h>

#include "MLX90641_API.h"
#include "MLX90641_I2C_Driver.h"
/* DEBUG
#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial

#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
#define debug  Serial
#else
#define debug  Serial
#endif
*/

#define TA_SHIFT 5 //Default shift for MLX90641 in open air, TEST 8

const byte MLX90641_address = 0x33; //Default 7-bit unshifted address of the MLX90641
static float mlx90641To[192];
paramsMLX90641 mlx90641;
unsigned long myTime;


void setup() {
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz
  Serial.begin(115200);
  /*debug.begin(9600);
  while (!debug); //Wait for user to open terminal
  debug.println("MLX90641 IR Array Example");
  */
  
  if (isConnected() == false) {
    //debug.println("MLX90641 not detected at default I2C address. Please check wiring. Freezing.");
    Serial.print("ERR001");
    while (1);
  }
  //debug.println("MLX90641 online!");

  int status;
  uint16_t eeMLX90641[832];
  status = MLX90641_DumpEE(MLX90641_address, eeMLX90641);
  if (status != 0) {
    //debug.println("Failed to load system parameters");
    Serial.println("ERR002");
  }
  //debug.println("load parameters success");
  status = MLX90641_ExtractParameters(eeMLX90641, &mlx90641);
  if (status != 0) {
    //debug.println("Parameter extraction failed");
    Serial.println("ERR003");
  }
  //debug.println("parameter extraction success");

  /*
   * 0x05-0.5Hz; 0x01-1Hz; 0x02-2Hz; 0x03-4Hz; 0x04-8Hz(DEFAULT); 0x05-16Hz; 0x06-32Hz;0x07-64Hz 
  */
  MLX90641_SetRefreshRate(MLX90641_address, 0x05); //Set rate to 8Hz effective - Works
  
  //eeMLX90641 array only needed for params extraction 
}

void loop() {
  for (byte x = 0 ; x < 2 ; x++) { //Read both subpages
    uint16_t mlx90641Frame[242]; //242
    int status = MLX90641_GetFrameData(MLX90641_address, mlx90641Frame);
    // DEBUG 0/1 SUCCESS debug.println("GetFrameData" + String(status));
    if (status < 0) {
      Serial.print("GetFrame Error: ");
      Serial.println(status);
    }

    float vdd = MLX90641_GetVdd(mlx90641Frame, &mlx90641);
    // DEBUG 3.3(V) SUCCESS
    //debug.println(vdd);
    float Ta = MLX90641_GetTa(mlx90641Frame, &mlx90641);
    // DEBUG ambient temp SUCCESS
    //debug.println(Ta);
    if (x==0){
      /*Serial.print("Time: ");
      myTime = millis();
      Serial.println(myTime);*/
      
      Serial.println(vdd);
      Serial.println(Ta);
    }
    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    float emissivity = 0.95;

    MLX90641_CalculateTo(mlx90641Frame, &mlx90641, emissivity, tr, mlx90641To);
  }
 
  /*for (int i = 0; i < 192; i++)
  {
    debug.print(mlx90641To[i]);
  }
*/
    int count = 0;
    for (int x = 0 ; x < 192 ; x++) {
        if(count>15){
          Serial.print("\n");
          count = 0;
        }
        Serial.print(mlx90641To[x], 2); // Adjust sig figs for temperature
        Serial.print(",");
        count++;
    }
    Serial.println("");  
    /*Serial.print("End time: ");
    myTime = millis();
    Serial.println(myTime); */
  
  //delay(5000);
}

//Returns true if the MLX90641 is detected on the I2C bus
boolean isConnected() {
  Wire.beginTransmission((uint8_t)MLX90641_address);
  if (Wire.endTransmission() != 0) {
    return (false);    //Sensor NACK
  }
  return (true);
}
