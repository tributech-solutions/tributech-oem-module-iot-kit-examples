/*
 * dps368.h
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */

#ifndef DPS368_H_
#define DPS368_H_

#include <kernel.h>
#include <init.h>
#include <sys/byteorder.h>
#include <sys/util.h>

#include <drivers/i2c.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// temperature oversampling rate definition
#define CONFIG_DPS368_TEMP_OSR_2X

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// pressure oversampling rate definition
#define CONFIG_DPS368_PRESS_OSR_2X


/* Register addresses as in the datasheet */
#define IFX_DPS368_REG_ADDR_PSR_B2			(0x00)
#define IFX_DPS368_REG_ADDR_TMP_B2			(0x03)
#define IFX_DPS368_REG_ADDR_PRS_CFG			(0x06)
#define IFX_DPS368_REG_ADDR_TMP_CFG			(0x07)
#define IFX_DPS368_REG_ADDR_MEAS_CFG			(0x08)
#define IFX_DPS368_REG_ADDR_CFG_REG			(0x09)
#define IFX_DPS368_REG_ADDR_INT_STS			(0x0A)
#define IFX_DPS368_REG_ADDR_FIFO_STS			(0x0B)
#define IFX_DPS368_REG_ADDR_RESET			(0x0C)
#define IFX_DPS368_REG_ADDR_PRODUCT_ID			(0x0D)
#define IFX_DPS368_REG_ADDR_COEF_0			(0x10)
#define IFX_DPS368_REG_ADDR_COEF_SRCE			(0x28)

enum {
    IFX_DPS368_MODE_IDLE			=	0x00,
    IFX_DPS368_MODE_COMMAND_PRESSURE            =	0x01,
    IFX_DPS368_MODE_COMMAND_TEMPERATURE         =	0x02,
    IFX_DPS368_MODE_BACKGROUND_PRESSURE         =	0x05,
    IFX_DPS368_MODE_BACKGROUND_TEMPERATURE	=	0x06,
    IFX_DPS368_MODE_BACKGROUND_ALL		=	0x07
};

/* Bits in registers as in the datasheet */
#define IFX_DPS368_REG_ADDR_MEAS_CFG_PRS_RDY		(0x10)
#define IFX_DPS368_REG_ADDR_MEAS_CFG_TMP_RDY		(0x20)

/*
 * If sensor is ready after self initialization
 * bits 6 and 7 in register MEAS_CFG (0x08) should be "1"
 */
#define IFX_DPS368_REG_ADDR_MEAS_CFG_SELF_INIT_OK	(0xC0)
#define IFX_DPS368_COEF_SRCE_MASK			(0x80)
#define IFX_DPS368_PRODUCT_ID				(0x10)

/* Polling time in ms*/
#define POLL_TIME_MS					(K_MSEC(10))
/* Number of times to poll before timeout */
#define POLL_TRIES					3

/*
 * Measurement times in ms for different oversampling rates
 * From Table 16 in the datasheet, rounded up for safety margin
 */
enum {
	IFX_DPS368_MEAS_TIME_OSR_1	= 4,
	IFX_DPS368_MEAS_TIME_OSR_2	= 6,
	IFX_DPS368_MEAS_TIME_OSR_4	= 9,
	IFX_DPS368_MEAS_TIME_OSR_8	= 15,
	IFX_DPS368_MEAS_TIME_OSR_16	= 28,
	IFX_DPS368_MEAS_TIME_OSR_32	= 54,
	IFX_DPS368_MEAS_TIME_OSR_64	= 105,
	IFX_DPS368_MEAS_TIME_OSR_128	= 207
};

/* Compensation scale factors from Table 9 in the datasheet */
enum {
	IFX_DPS368_SF_OSR_1		= 524288,
	IFX_DPS368_SF_OSR_2		= 1572864,
	IFX_DPS368_SF_OSR_4		= 3670016,
	IFX_DPS368_SF_OSR_8		= 7864320,
	IFX_DPS368_SF_OSR_16		= 253952,
	IFX_DPS368_SF_OSR_32		= 516096,
	IFX_DPS368_SF_OSR_64		= 1040384,
	IFX_DPS368_SF_OSR_128		= 2088960
};

/*
 * Oversampling and measurement rates configuration for pressure and temperature
 * sensor According to Table 16 of the datasheet
 */
enum {
	IFX_DPS368_RATE_1		= 0x00,
	IFX_DPS368_RATE_2		= 0x01,
	IFX_DPS368_RATE_4		= 0x02,
	IFX_DPS368_RATE_8		= 0x03,
	IFX_DPS368_RATE_16		= 0x04,
	IFX_DPS368_RATE_32		= 0x05,
	IFX_DPS368_RATE_64		= 0x06,
	IFX_DPS368_RATE_128		= 0x07
};

/* Helper macro to set temperature and pressure config register */
#define CFG_REG(MEAS_RATE, OSR_RATE)                                           \
	((((MEAS_RATE)&0x07) << 4) | ((OSR_RATE)&0x07))


        /* Setup constants depending on temperature oversampling factor */
#if defined CONFIG_DPS368_TEMP_OSR_1X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_1
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_1
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_1)
#define IFX_DPS368_T_SHIFT		0
#elif defined CONFIG_DPS368_TEMP_OSR_2X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_2
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_2
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_2)
#define IFX_DPS368_T_SHIFT		0
#elif defined CONFIG_DPS368_TEMP_OSR_4X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_4
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_4
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_4)
#define IFX_DPS368_T_SHIFT		0
#elif defined CONFIG_DPS368_TEMP_OSR_8X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_8
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_8
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_8)
#define IFX_DPS368_T_SHIFT		0
#elif defined CONFIG_DPS368_TEMP_OSR_16X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_16
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_16
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_16)
#define IFX_DPS368_T_SHIFT		1
#elif defined CONFIG_DPS368_TEMP_OSR_32X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_32
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_32
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_32)
#define IFX_DPS368_T_SHIFT		1
#elif defined CONFIG_DPS368_TEMP_OSR_64X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_64
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_64
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_64)
#define IFX_DPS368_T_SHIFT		1
#elif defined CONFIG_DPS368_TEMP_OSR_128X
#define IFX_DPS368_SF_TMP		IFX_DPS368_SF_OSR_128
#define IFX_DPS368_TMP_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_128
#define IFX_DPS368_TMP_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_128)
#define IFX_DPS368_T_SHIFT		1
#endif

/* Setup constants depending on pressure oversampling factor */
#if defined CONFIG_DPS368_PRESS_OSR_1X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_1
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_1
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_1)
#define IFX_DPS368_P_SHIFT		0
#elif defined CONFIG_DPS368_PRESS_OSR_2X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_2
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_2
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_2)
#define IFX_DPS368_P_SHIFT		0
#elif defined CONFIG_DPS368_PRESS_OSR_4X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_4
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_4
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_4)
#define IFX_DPS368_P_SHIFT		0
#elif defined CONFIG_DPS368_PRESS_OSR_8X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_8
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_8
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_8)
#define IFX_DPS368_P_SHIFT		0
#elif defined CONFIG_DPS368_PRESS_OSR_16X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_16
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_16
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_16)
#define IFX_DPS368_P_SHIFT		1
#elif defined CONFIG_DPS368_PRESS_OSR_32X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_32
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_32
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_32)
#define IFX_DPS368_P_SHIFT		1
#elif defined CONFIG_DPS368_PRESS_OSR_64X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_64
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_64
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_64)
#define IFX_DPS368_P_SHIFT		1
#elif defined CONFIG_DPS368_PRESS_OSR_128X
#define IFX_DPS368_SF_PSR		IFX_DPS368_SF_OSR_128
#define IFX_DPS368_PSR_MEAS_TIME	IFX_DPS368_MEAS_TIME_OSR_128
#define IFX_DPS368_PSR_CFG		CFG_REG(IFX_DPS368_RATE_1, IFX_DPS368_RATE_128)
#define IFX_DPS368_P_SHIFT		1
#endif

#define DPS368_CFG_REG                                                         \
	(((IFX_DPS368_T_SHIFT & 0x01) << 3) |                                  \
	 ((IFX_DPS368_P_SHIFT & 0x01) << 2))

#define HW_BUG_FIX_SEQUENCE_LEN	5

#define POW_2_23_MINUS_1	BIT_MASK(23)
#define POW_2_24		BIT(24)
#define POW_2_15_MINUS_1	BIT_MASK(15)
#define POW_2_16		BIT(16)
#define POW_2_11_MINUS_1	BIT_MASK(11)
#define POW_2_12		BIT(12)
#define POW_2_20		BIT(20)
#define POW_2_19_MINUS_1	BIT_MASK(19)

/* Needed because the values are referenced by pointer. */
static const uint8_t REG_ADDR_MEAS_CFG = IFX_DPS368_REG_ADDR_MEAS_CFG;
static const uint8_t REG_ADDR_CALIB_COEFF_0 = IFX_DPS368_REG_ADDR_COEF_0;
static const uint8_t REG_ADDR_TMP_B2 = IFX_DPS368_REG_ADDR_TMP_B2;
static const uint8_t REG_ADDR_PSR_B2 = IFX_DPS368_REG_ADDR_PSR_B2;
static const uint8_t REG_ADDR_COEF_SRCE = IFX_DPS368_REG_ADDR_COEF_SRCE;

/* calibration coefficients */
struct dps368_cal_coeff {
	/* Pressure Sensor Calibration Coefficients */
	int32_t c00; /* 20bit */
	int32_t c10; /* 20bit */
	int16_t c01; /* 16bit */
	int16_t c11; /* 16bit */
	int16_t c20; /* 16bit */
	int16_t c21; /* 16bit */
	int16_t c30; /* 16bit */
	/* Temperature Sensor Calibration Coefficients */
	int16_t c0; /* 12bit */
	int16_t c1; /* 12bit */
};

typedef struct {
	const struct device *i2c_master;
	struct dps368_cal_coeff comp;
	/* Temperature Values */
	int32_t tmp_val1;
	int32_t tmp_val2;
        /* Temperature value in float*/
        float tmp_val;
	/* Last raw temperature value for temperature compensation */
	int32_t raw_tmp;
	/* Pressure Values */
	int32_t psr_val1;
	int32_t psr_val2;
        /* Pressure value in float*/
        float psr_val;
}dps368_data_t;

extern dps368_data_t data;

typedef struct {
	char *i2c_bus_name;
	uint16_t i2c_addr;
}dps368_cfg_t;

extern dps368_cfg_t dp368_config;

/*
 * Convert the bytes from calibration memory to calibration coefficients
 * structure
 */
void dps368_calib_coeff_creation(const uint8_t raw_coef[18], struct dps368_cal_coeff *comp);

/* Poll one or multiple bits given by ready_mask in reg_addr */
bool poll_rdy(uint8_t reg_addr, uint8_t ready_mask);

/* Trigger a temperature measurement and wait until the result is stored */
bool dps368_trigger_temperature();

/* Trigger a pressure measurement and wait until the result is stored */
bool dps368_trigger_pressure();

/*
 * function to fix a hardware problem on some devices
 * you have this bug if you measure around 60°C when temperature is around 20°C
 * call dps368_hw_bug_fix() directly in the init() function to fix this issue
 */
void dps368_hw_bug_fix();

/*
 * Scale and compensate the raw temperature measurement value to micro °C
 * The formula is based on the Chapter 4.9.2 in the datasheet.
 */
void dps368_scale_temperature(int32_t tmp_raw);

/*
 * Scale and temperature compensate the raw pressure measurement value to
 * Kilopascal. The formula is based on the Chapter 4.9.1 in the datasheet.
 */
void dps368_scale_pressure(int32_t tmp_raw, int32_t psr_raw);

/* Convert the raw sensor data to int32_t */
int32_t raw_to_int24(const uint8_t raw[3]);

/* perform a single measurement of temperature and pressure */
bool dps368_measure_tmp_psr();

/*
 * perform a single pressure measurement
 * uses the stored temperature value for sensor temperature compensation
 * temperature must be measured regularly for good temperature compensation
 */
bool dps368_measure_psr();

/* perform a single temperature measurement */
bool dps368_measure_tmp();

/* Initialize the sensor and apply the configuration */
int dps368_init();


#endif /* DPS368_H_ */