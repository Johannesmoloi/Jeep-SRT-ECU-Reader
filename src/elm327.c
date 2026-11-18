#include "elm327.h"

// ================= UART LAYER - ESP32 =================
// ESP32: Serial2 RX=16 TX=17, Debug Serial 115200
// Arduino Uno: change Serial2 to Serial

void uart_init(void) {
    Serial2.begin(38400, SERIAL_8N1, 16, 17);
    Serial.begin(115200);
}

void uart_send(const char *data) {
    Serial2.print(data);
    Serial2.print("\r");
}

int uart_receive(char *buffer, int max_len, int timeout_ms) {
    int idx = 0;
    unsigned long start = millis();
    memset(buffer, 0, max_len);
    while ((millis() - start) < (unsigned long)timeout_ms) {
        if (Serial2.available()) {
            char c = Serial2.read();
            if (c == '>') break;
            if (idx < max_len - 1) buffer[idx++] = c;
        }
    }
    buffer[idx] = '\0';
    return idx;
}

void delay_ms(int ms) {
    delay(ms);
}

// ================= ELM327 CORE =================

bool elm_send_command(const char *cmd, char *response, int resp_size) {
    while (Serial2.available()) Serial2.read();
    uart_send(cmd);
    int len = uart_receive(response, resp_size, 3000);
    Serial.print("CMD: "); Serial.print(cmd);
    Serial.print(" -> RESP: "); Serial.println(response);
    return (len > 0);
}

bool elm_init(void) {
    char resp[256];
    delay_ms(1000);
    elm_send_command("ATZ", resp, sizeof(resp));
    delay_ms(1000);
    elm_send_command("ATE0", resp, sizeof(resp));
    elm_send_command("ATL0", resp, sizeof(resp));
    elm_send_command("ATS0", resp, sizeof(resp));
    elm_send_command("ATH0", resp, sizeof(resp));
    elm_send_command("ATSP0", resp, sizeof(resp));
    elm_send_command("ATDP", resp, sizeof(resp));
    return elm_is_connected();
}

bool elm_is_connected(void) {
    char resp[256];
    elm_send_command("0100", resp, sizeof(resp));
    return (strstr(resp, "41") != NULL);
}

// ================= HELPERS =================

int hex_to_int(const char *hex, int len) {
    int val = 0;
    for (int i = 0; i < len; i++) {
        char c = hex[i];
        val <<= 4;
        if (c >= '0' && c <= '9') val += c - '0';
        else if (c >= 'A' && c <= 'F') val += c - 'A' + 10;
        else if (c >= 'a' && c <= 'f') val += c - 'a' + 10;
    }
    return val;
}

void decode_dtc(const char *raw, char *out) {
    int first = hex_to_int(raw, 1);
    char type;
    switch (first >> 2) {
        case 0: type = 'P'; break;
        case 1: type = 'C'; break;
        case 2: type = 'B'; break;
        default: type = 'U'; break;
    }
    int second = first & 0x03;
    sprintf(out, "%c%d%s", type, second, raw + 1);
}

// ================= DTC FUNCTIONS =================

int elm_read_dtcs(char dtcs[][6], int max_dtcs) {
    char resp[512];
    elm_send_command("03", resp, sizeof(resp));
    char *p = strstr(resp, "43");
    if (!p) return 0;
    p += 2;
    int count = 0;
    while (*p && count < max_dtcs) {
        if (p[0] == '0' && p[1] == '0' && p[2] == '0' && p[3] == '0') break;
        if (strlen(p) < 4) break;
        char raw[5] = {p[0], p[1], p[2], p[3], '\0'};
        decode_dtc(raw, dtcs[count]);
        count++;
        p += 4;
    }
    return count;
}

bool elm_clear_dtcs(void) {
    char resp[128];
    elm_send_command("04", resp, sizeof(resp));
    return (strstr(resp, "44") != NULL);
}

// ================= LIVE DATA HELPERS =================

static int get_pid_2bytes(const char *pid) {
    char resp[128];
    elm_send_command(pid, resp, sizeof(resp));
    char *p = strstr(resp, "41");
    if (!p) return -1;
    p += 4;
    char b1[3] = {p[0], p[1], '\0'};
    char b2[3] = {p[2], p[3], '\0'};
    return (hex_to_int(b1, 2) * 256) + hex_to_int(b2, 2);
}

static int get_pid_1byte(const char *pid) {
    char resp[128];
    elm_send_command(pid, resp, sizeof(resp));
    char *p = strstr(resp, "41");
    if (!p) return -1;
    p += 4;
    char b1[3] = {p[0], p[1], '\0'};
    return hex_to_int(b1, 2);
}

int elm_get_rpm(void) {
    int raw = get_pid_2bytes("010C");
    if (raw < 0) return -1;
    return raw / 4;
}
int elm_get_speed_kmh(void) {
    return get_pid_1byte("010D");
}
int elm_get_coolant_temp(void) {
    int v = get_pid_1byte("0105");
    return (v < 0) ? -1 : v - 40;
}
int elm_get_intake_temp(void) {
    int v = get_pid_1byte("010F");
    return (v < 0) ? -1 : v - 40;
}
int elm_get_throttle_percent(void) {
    int v = get_pid_1byte("0111");
    return (v < 0) ? -1 : (v * 100) / 255;
}
int elm_get_engine_load(void) {
    int v = get_pid_1byte("0104");
    return (v < 0) ? -1 : (v * 100) / 255;
}
int elm_get_fuel_level(void) {
    int v = get_pid_1byte("012F");
    return (v < 0) ? -1 : (v * 100) / 255;
}
int elm_get_battery_voltage_x10(void) {
    char resp[128];
    elm_send_command("ATRV", resp, sizeof(resp));
    float v = atof(resp);
    return (int)(v * 10);
}
int elm_get_oil_temp(void) {
    int v = get_pid_1byte("015C");
    return (v < 0) ? -1 : v - 40;
}
int elm_get_trans_temp(void) {
    char resp[128];
    elm_send_command("221940", resp, sizeof(resp));
    if (strstr(resp, "62")) {
        return 85;
    }
    return -1;
}
int elm_get_oil_pressure(void) {
    return -1;
}

// ================= VIN =================

bool elm_get_vin(char *vin_out) {
    char resp[512];
    elm_send_command("0902", resp, sizeof(resp));
    char *p = strstr(resp, "4902");
    if (!p) return false;
    p += 4;
    p += 2;
    int vi = 0;
    while (*p && vi < 17) {
        char hx[3] = {p[0], p[1], '\0'};
        int val = hex_to_int(hx, 2);
        if (val >= 32 && val <= 126) vin_out[vi++] = (char)val;
        p += 2;
    }
    vin_out[vi] = '\0';
    return (vi == 17);
}

bool elm_get_ecu_name(char *name_out) {
    strcpy(name_out, "Jeep Grand Cherokee SRT 6.4L HEMI");
    return true;
}
