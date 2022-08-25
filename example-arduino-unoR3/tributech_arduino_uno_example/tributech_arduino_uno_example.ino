/*
 Tributech OEM Arduino Example with DPS310 sensor
*/

#include <Adafruit_DPS310.h>
#include "base64.hpp"

int counter;
int transaction_number =  0;
float temperature_previous;
float pressure_previous;

char base64_string[50]; 
char valuemetadataid_temperature[37] = "fd54f5ca-15c9-4e01-b4b3-4247198a846c"; 
char valuemetadataid_pressure[37] = "4e8e64f4-5439-4676-85c5-117280901bc0";
char provide_values_message[500];

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

    transaction_number += 1;
    if (transaction_number > 3000)
    {
      transaction_number = 1;
    }
    sprintf(provide_values_message, "{\"TransactionNr\": %d,\"Operation\": \"ProvideValues\",\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}]}\r\n" , transaction_number, valuemetadataid_temperature, base64_string);
    Serial.println(provide_values_message);

    temperature_previous = temp_event.temperature;
    counter = 0;
  }

  //+++++++++++++++++++++++++++++++++++++++++++++++++++++
  // send pressure
  if (counter > 5000 && (pressure_event.pressure > pressure_previous + 1 || pressure_event.pressure < pressure_previous - 1))
  {
    encode_base64((char *) &pressure_event.pressure, 4, base64_string);

    transaction_number += 1;
    if (transaction_number > 3000)
    {
      transaction_number = 1;
    }
    sprintf(provide_values_message, "{\"TransactionNr\": %d,\"Operation\": \"ProvideValues\",\"ValueMetadataId\": \"%s\",\"Values\": [{\"Timestamp\": 0,\"Value\": \"%s\"}]}\r\n" , transaction_number, valuemetadataid_pressure, base64_string);
    Serial.println(provide_values_message);

    pressure_previous = pressure_event.pressure;
    counter = 0;
  } 
}

