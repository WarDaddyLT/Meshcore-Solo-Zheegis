#pragma once

#include <MeshCore.h>
#include <Arduino.h>
#include <helpers/NRF52Board.h>
#ifdef TECHO_STATUS_LEDS
#include <helpers/ui/LedBlinkBurst.h>
#endif

// ============================================================
// T-Echo Lite battery pins — hardcoded from LilyGo t_echo_lite_config.h
// NOT using any defines from variant.h for battery measurement
// ============================================================
#define PIN_VBAT_READ         _PINNUM(0, 2)   // BATTERY_ADC_DATA
#define PIN_VBAT_MEAS_EN      _PINNUM(0, 31)  // BATTERY_MEASUREMENT_CONTROL

class TechoBoard : public NRF52BoardDCDC {
#ifdef TECHO_STATUS_LEDS
  LedBlinkBurst status_led, gps_led;
#endif
public:
#ifdef TECHO_STATUS_LEDS
  void onStatusTransmitted() { status_led.trigger(millis()); }
  void onGPSFix() { gps_led.trigger(millis()); }
  void updateTrackerLeds() {
    digitalWrite(LED_BLUE, status_led.isOn(millis()) ? LED_STATE_ON : !LED_STATE_ON);
    digitalWrite(LED_GREEN, gps_led.isOn(millis()) ? LED_STATE_ON : !LED_STATE_ON);
  }
#endif
  TechoBoard() : NRF52Board("TECHO_OTA") {}
  void begin();
  uint16_t getBattMilliVolts() override;

  const char* getManufacturerName() const override {
    return "LilyGo T-Echo Lite";
  }

  #if defined(P_LORA_TX_LED)
  void onBeforeTransmit() override {
    digitalWrite(P_LORA_TX_LED, LOW);   // turn TX LED on
  }

  void onAfterTransmit() override {
    digitalWrite(P_LORA_TX_LED, HIGH);   // turn TX LED off
  }
  #endif

  void shutdownPeripherals() override {
    NRF52Board::shutdownPeripherals();

    digitalWrite(PIN_VBAT_MEAS_EN, LOW);
    #ifdef LED_RED
    digitalWrite(LED_RED, LOW);
    #endif
    #ifdef LED_GREEN
#ifdef TECHO_STATUS_LEDS
    digitalWrite(LED_GREEN, !LED_STATE_ON);
#else
    digitalWrite(LED_GREEN, LOW);
#endif
    #endif
    #ifdef LED_BLUE
#ifdef TECHO_STATUS_LEDS
    digitalWrite(LED_BLUE, !LED_STATE_ON);
#else
    digitalWrite(LED_BLUE, LOW);
#endif
    #endif
    #ifdef DISP_BACKLIGHT
    digitalWrite(DISP_BACKLIGHT, LOW);
    #endif
    #ifdef PIN_PWR_EN
    digitalWrite(PIN_PWR_EN, LOW);
    #endif
  }
};