#include "elm327.h"

void print_menu(void) {
    Serial.println("\n========== JEEP SRT ECU READER ==========");
    Serial.println("1 - Check ECU Connection");
    Serial.println("2 - Read Fault Codes (P/B/C/U)");
    Serial.println("3 - Clear Fault Codes");
    Serial.println("4 - Live Data - SRT Dashboard (10x)");
    Serial.println("5 - Read VIN");
    Serial.println("6 - Battery + Temps");
    Serial.println("0 - Show Menu Again");
    Serial.println("Enter choice: ");
}

void show_live_data(void) {
    Serial.println("\n--- SRT LIVE DATA ---");
    Serial.print("RPM: "); Serial.println(elm_get_rpm());
    Serial.print("Speed km/h: "); Serial.println(elm_get_speed_kmh());
    Serial.print("Coolant C: "); Serial.println(elm_get_coolant_temp());
    Serial.print("Intake Temp C: "); Serial.println(elm_get_intake_temp());
    Serial.print("Throttle %: "); Serial.println(elm_get_throttle_percent());
    Serial.print("Engine Load %: "); Serial.println(elm_get_engine_load());
    Serial.print("Oil Temp C: "); Serial.println(elm_get_oil_temp());
    Serial.print("Fuel Level %: "); Serial.println(elm_get_fuel_level());
    int batt = elm_get_battery_voltage_x10();
    Serial.print("Battery V: "); Serial.print(batt / 10); Serial.print("."); Serial.println(batt % 10);
}

void setup() {
    uart_init();
    delay_ms(500);
    Serial.println("Jeep SRT ECU Reader Starting...");
    if (elm_init()) {
        Serial.println("ECU CONNECTED - Jeep SRT Found");
    } else {
        Serial.println("ECU NOT FOUND - Check ELM327, Ignition ON");
    }
    print_menu();
}

void loop() {
    if (Serial.available()) {
        char choice = Serial.read();
        if (choice == '1') {
            if (elm_is_connected()) Serial.println("ECU: CONNECTED");
            else Serial.println("ECU: NOT CONNECTED");
        }
        else if (choice == '2') {
            char dtcs[20][6];
            int n = elm_read_dtcs(dtcs, 20);
            Serial.print("Fault Codes Found: "); Serial.println(n);
            if (n == 0) Serial.println("No Codes - System OK");
            for (int i = 0; i < n; i++) {
                Serial.print(" - "); Serial.println(dtcs[i]);
            }
        }
        else if (choice == '3') {
            Serial.println("Clearing codes... Turn engine OFF, Ignition ON");
            if (elm_clear_dtcs()) Serial.println("Codes Cleared!");
            else Serial.println("Clear Failed");
        }
        else if (choice == '4') {
            for (int i = 0; i < 10; i++) {
                show_live_data();
                delay_ms(1000);
            }
        }
        else if (choice == '5') {
            char vin[20] = {0};
            if (elm_get_vin(vin)) {
                Serial.print("VIN: "); Serial.println(vin);
            } else Serial.println("VIN Read Failed");
        }
        else if (choice == '6') {
            show_live_data();
        }
        else if (choice == '0') {
            // just show menu again below
        }
        print_menu();
    }
}
