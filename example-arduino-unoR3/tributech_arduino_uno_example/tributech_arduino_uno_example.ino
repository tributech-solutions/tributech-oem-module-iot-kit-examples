/*
 Tributech OEM Arduino Example with DPS310 sensor
*/

#include <Adafruit_DPS310.h>
#include "base64.hpp"

extern "C"{
  #include "tributech_oem_api.h"
}

int counter;
float temperature_previous;
float pressure_previous;

char base64_string[50]; 
char valuemetadataid_temperature[37] = "3b619323-7a61-465b-88df-24297efd5dda"; 
char valuemetadataid_pressure[37] = "dbc298ea-f9b2-4daf-a93b-5a891fd4ddc1";
char provide_value_message[500];

Adafruit_DPS310 dps;
Adafruit_Sensor *dps_temp = dps.getTemperatureSensor();
Adafruit_Sensor *dps_pressure = dps.getPressureSensor();


void setup() 
{
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // initialize serial:
  Serial.begin(115200);

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  //Serial.println("DPS310");
  if (! dps.begin_I2C()) {
    Serial.println("Failed to find DPS");
    while (1) yield();
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // Setup highest precision
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
}

void loop() 
{
  sensors_event_t temp_event, pressure_event;
  
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // increase counter
  counter +=1;

  if (counter > 30000)
  {
    counter = 0;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // get temp and pressure events
  if (dps.temperatureAvailable()) {
    dps_temp->getEvent(&temp_event);
  }

  // Reading pressure also reads temp so don't check pressure
  // before temp!
  if (dps.pressureAvailable()) {
    dps_pressure->getEvent(&pressure_event);
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // send temperature
  if (counter > 5000 && (temp_event.temperature > temperature_previous + 1 || temp_event.temperature < temperature_previous - 1))
  {
    encode_base64((char *) &temp_event.temperature, 4, base64_string);

    increase_transaction_nr();
    build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_temperature,base64_string,"0");
    Serial.println(provide_value_message);

    temperature_previous = temp_event.temperature;
    counter = 0;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // send pressure
  if (counter > 5000 && (pressure_event.pressure > pressure_previous + 1 || pressure_event.pressure < pressure_previous - 1))
  {
    encode_base64((char *) &pressure_event.pressure, 4, base64_string);

    increase_transaction_nr();
    build_provide_value(provide_value_message,transaction_nr_string,valuemetadataid_pressure,base64_string,"0");
    Serial.println(provide_value_message);

    pressure_previous = pressure_event.pressure;
    counter = 0;
  } 
}
