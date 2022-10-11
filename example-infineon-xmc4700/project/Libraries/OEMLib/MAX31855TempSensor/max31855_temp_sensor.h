#ifndef MX31855_TEMP_SENSOR_H_
#define MX31855_TEMP_SENSOR_H_

#include <DAVE.h>                 //Declarations from DAVE Code Generation (includes SFR declaration)
#include <functions.h>
#include <SPI_MASTER/spi_master.h>

extern float max31855_temp_external;
extern float max31855_temp_internal;

int get_max31855_temp(SPI_MASTER_t *const handle, const DIGITAL_IO_t *const cs, float* external_temp, float* internal_temp);

#endif
