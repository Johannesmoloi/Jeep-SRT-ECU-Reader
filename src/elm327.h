#ifndef ELM327_H
#define ELM327_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

void uart_init(void);
void uart_send(const char *data);
int  uart_receive(char *buffer, int max_len, int timeout_ms);
void delay_ms(int ms);

bool elm_init(void);
bool elm_send_command(const char *cmd, char *response, int resp_size);
bool elm_is_connected(void);

int  elm_read_dtcs(char dtcs[][6], int max_dtcs);
bool elm_clear_dtcs(void);

int elm_get_rpm(void);
int elm_get_speed_kmh(void);
int elm_get_coolant_temp(void);
int elm_get_intake_temp(void);
int elm_get_throttle_percent(void);
int elm_get_engine_load(void);
int elm_get_battery_voltage_x10(void);
int elm_get_fuel_level(void);

int elm_get_oil_temp(void);
int elm_get_trans_temp(void);
int elm_get_oil_pressure(void);

bool elm_get_vin(char *vin_out);
bool elm_get_ecu_name(char *name_out);

int hex_to_int(const char *hex, int len);
void decode_dtc(const char *raw, char *out);

#endif
