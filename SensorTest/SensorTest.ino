/*
    Read the temperature pixels from the MLX90641 IR array
*/

#include <Wire.h>

#include "MLX90641_API.h"
#include "MLX90641_I2C_Driver.h"

#if defined(ARDUINO_ARCH_AVR)
#define debug  Serial

#elif defined(ARDUINO_ARCH_SAMD) ||  defined(ARDUINO_ARCH_SAM)
#define debug  Serial
#else
#define debug  Serial
#endif




const byte MLX90641_address = 0x33; //Default 7-bit unshifted address of the MLX90641

#define TA_SHIFT 5 //Default shift for MLX90641 in open air, TEST 8

static float mlx90641To[192];
paramsMLX90641 mlx90641;

void setup() {
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz

  debug.begin(9600);
  while (!debug); //Wait for user to open terminal
  debug.println("MLX90641 IR Array Example");

  if (isConnected() == false) {
    debug.println("MLX90641 not detected at default I2C address. Please check wiring. Freezing.");
    while (1);
  }
  debug.println("MLX90641 online!");

  //Get device parameters - We only have to do this once
  int status;
  uint16_t eeMLX90641[832];
  status = MLX90641_DumpEE(MLX90641_address, eeMLX90641);
  if (status != 0) {
    debug.println("Failed to load system parameters");
  }
  debug.println("load parameters success");
  status = MLX90641_ExtractParameters(eeMLX90641, &mlx90641);
  if (status != 0) {
    debug.println("Parameter extraction failed");
  }
  debug.println("parameter extraction success");

  //Once params are extracted, we can release eeMLX90641 array
}

void loop() {
  for (byte x = 0 ; x < 2 ; x++) { //Read both subpages
    uint16_t mlx90641Frame[242]; //242
    int status = MLX90641_GetFrameData(MLX90641_address, mlx90641Frame);
    // DEBUG 0/1 SUCCESS debug.println("GetFrameData" + String(status));
    if (status < 0) {
      debug.print("GetFrame Error: ");
      debug.println(status);
    }

    float vdd = MLX90641_GetVdd(mlx90641Frame, &mlx90641);
    // DEBUG 3.3(V) SUCCESS
    debug.println(vdd);
    float Ta = MLX90641_GetTa(mlx90641Frame, &mlx90641);
    // DEBUG ambient temp SUCCESS
    debug.println(Ta);
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
          debug.print("\n");
          count = 0;
        }
        debug.print(mlx90641To[x], 2);
        debug.print(",");
        count++;
    }
    debug.println("");  
  
  delay(5000);
}

//Returns true if the MLX90641 is detected on the I2C bus
boolean isConnected() {
  Wire.beginTransmission((uint8_t)MLX90641_address);
  if (Wire.endTransmission() != 0) {
    return (false);    //Sensor did not ACK
  }
  return (true);
}
