// Adafruit NXP Precision 9DoF breakout (FXOS8700 accel/mag + FXAS21002 gyro)
// EVA running at 16 MHz external clock
// Hardware:
// PC0 (SCL) -> breakout SCL
// PC1 (SDA) -> breakout SDA
// 5V -> VIN, GND -> GND (breakout has its own regulator, level shifting and pull-ups)
// TWI is polled (no ISR), so it does not interfere with the odometry interrupts.

#ifndef IMU_HW_H
#define IMU_HW_H

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/twi.h>

#define TWI_FREQ        100000UL    // 100 kHz; both chips also handle 400 kHz
#define TWI_TIMEOUT     10000       // poll iterations (~5 ms) before giving up

// I2C addresses on the Adafruit board
#define FXOS8700_ADDR   0x1F
#define FXAS21002_ADDR  0x21

// FXOS8700 registers
#define FXOS_STATUS         0x00
#define FXOS_WHO_AM_I       0x0D    // should read 0xC7
#define FXOS_XYZ_DATA_CFG   0x0E
#define FXOS_CTRL_REG1      0x2A
#define FXOS_CTRL_REG2      0x2B
#define FXOS_M_CTRL_REG1    0x5B
#define FXOS_M_CTRL_REG2    0x5C
#define FXOS_ID             0xC7

// FXAS21002 registers
#define FXAS_OUT_X_MSB      0x01
#define FXAS_WHO_AM_I       0x0C    // should read 0xD7
#define FXAS_CTRL_REG0      0x0D
#define FXAS_CTRL_REG1      0x13
#define FXAS_ID             0xD7

typedef struct {
    int16_t ax, ay, az;     // accel raw, 14-bit, 0.244 mg/LSB (+-2 g)
    int16_t mx, my, mz;     // mag raw, 0.1 uT/LSB
    int16_t gx, gy, gz;     // gyro raw, 7.8125 mdps/LSB (+-250 dps)
    uint8_t ok;             // 1 if both reads succeeded
} imu_sample_t;

// ---------- TWI (I2C) low level ----------

static uint8_t twi_wait(void) {
    uint16_t t = TWI_TIMEOUT;
    while (!(TWCR & (1<<TWINT))) {
        if (--t == 0) return 0;
    }
    return 1;
}

static uint8_t twi_start(uint8_t sla_rw) {
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);         // send (repeated) START
    if (!twi_wait()) return 0;
    if (TW_STATUS != TW_START && TW_STATUS != TW_REP_START) return 0;
    TWDR = sla_rw;                                  // address + R/W bit
    TWCR = (1<<TWINT)|(1<<TWEN);
    if (!twi_wait()) return 0;
    return (TW_STATUS == TW_MT_SLA_ACK) || (TW_STATUS == TW_MR_SLA_ACK);
}

static void twi_stop(void) {
    uint16_t t = TWI_TIMEOUT;
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
    while ((TWCR & (1<<TWSTO)) && --t);             // wait for STOP to finish
}

static uint8_t twi_write(uint8_t data) {
    TWDR = data;
    TWCR = (1<<TWINT)|(1<<TWEN);
    if (!twi_wait()) return 0;
    return TW_STATUS == TW_MT_DATA_ACK;
}

static uint8_t twi_read(uint8_t *data, uint8_t ack) {
    TWCR = (1<<TWINT)|(1<<TWEN)|(ack ? (1<<TWEA) : 0); // ACK all but last byte
    if (!twi_wait()) return 0;
    *data = TWDR;
    return 1;
}

static uint8_t imu_write_reg(uint8_t addr, uint8_t reg, uint8_t val) {
    uint8_t ok = twi_start((addr<<1)|TW_WRITE) && twi_write(reg) && twi_write(val);
    twi_stop();
    return ok;
}

static uint8_t imu_read_regs(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t n) {
    uint8_t ok = twi_start((addr<<1)|TW_WRITE) && twi_write(reg)
              && twi_start((addr<<1)|TW_READ);      // repeated START
    for (uint8_t i = 0; ok && i < n; i++) {
        ok = twi_read(&buf[i], i < n-1);
    }
    twi_stop();
    return ok;
}

// ---------- Public API ----------

// Returns 1 if both chips answered with the right WHO_AM_I, 0 otherwise.
uint8_t init_imu() {
    uint8_t id;

    TWSR = 0;                                       // prescaler 1
    TWBR = (uint8_t)((F_CPU/TWI_FREQ - 16) / 2);    // 72 -> 100 kHz
    TWCR = (1<<TWEN);

    // FXOS8700: accel + mag in hybrid mode
    if (!imu_read_regs(FXOS8700_ADDR, FXOS_WHO_AM_I, &id, 1) || id != FXOS_ID) return 0;
    imu_write_reg(FXOS8700_ADDR, FXOS_CTRL_REG1, 0x00);     // standby (needed to configure)
    imu_write_reg(FXOS8700_ADDR, FXOS_XYZ_DATA_CFG, 0x00);  // +-2 g
    imu_write_reg(FXOS8700_ADDR, FXOS_CTRL_REG2, 0x02);     // high resolution
    imu_write_reg(FXOS8700_ADDR, FXOS_M_CTRL_REG1, 0x1F);   // hybrid accel+mag, max oversampling
    imu_write_reg(FXOS8700_ADDR, FXOS_M_CTRL_REG2, 0x20);   // auto-increment 0x06 -> 0x33 (mag)
    imu_write_reg(FXOS8700_ADDR, FXOS_CTRL_REG1, 0x15);     // 100 Hz, low noise, active

    // FXAS21002: gyro
    if (!imu_read_regs(FXAS21002_ADDR, FXAS_WHO_AM_I, &id, 1) || id != FXAS_ID) return 0;
    imu_write_reg(FXAS21002_ADDR, FXAS_CTRL_REG1, 0x00);    // standby
    imu_write_reg(FXAS21002_ADDR, FXAS_CTRL_REG0, 0x03);    // +-250 dps
    imu_write_reg(FXAS21002_ADDR, FXAS_CTRL_REG1, 0x0E);    // 100 Hz, active

    _delay_ms(100);                                 // standby -> active settle time
    return 1;
}

imu_sample_t sample_imu() {
    imu_sample_t s = {0};
    uint8_t b[13];

    // One burst: STATUS, accel X/Y/Z (MSB,LSB), mag X/Y/Z (MSB,LSB)
    s.ok = imu_read_regs(FXOS8700_ADDR, FXOS_STATUS, b, 13);
    if (s.ok) {
        s.ax = (int16_t)(((uint16_t)b[1]<<8) | b[2]) >> 2;  // 14-bit, left-justified
        s.ay = (int16_t)(((uint16_t)b[3]<<8) | b[4]) >> 2;
        s.az = (int16_t)(((uint16_t)b[5]<<8) | b[6]) >> 2;
        s.mx = (int16_t)(((uint16_t)b[7]<<8)  | b[8]);
        s.my = (int16_t)(((uint16_t)b[9]<<8)  | b[10]);
        s.mz = (int16_t)(((uint16_t)b[11]<<8) | b[12]);
    }

    if (imu_read_regs(FXAS21002_ADDR, FXAS_OUT_X_MSB, b, 6)) {
        s.gx = (int16_t)(((uint16_t)b[0]<<8) | b[1]);
        s.gy = (int16_t)(((uint16_t)b[2]<<8) | b[3]);
        s.gz = (int16_t)(((uint16_t)b[4]<<8) | b[5]);
    } else {
        s.ok = 0;
    }
    return s;
}

// Integer unit conversions (avoids needing float printf on AVR)
static inline int32_t imu_accel_mg(int16_t raw)  { return ((int32_t)raw * 244) / 1000; }    // +-2 g
static inline int32_t imu_gyro_mdps(int16_t raw) { return ((int32_t)raw * 78125) / 10000; } // +-250 dps
// Mag: raw value is already in units of 0.1 uT

#endif