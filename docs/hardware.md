# Hardware integration reference

Žygis aims to support the hardware available to MeshCore Solo. This page records
current board adapters and their constraints; it does not define the project's
scope. See the [tracking guide](tracking.md) for shared behavior and the
[Solo hardware reference](../README-Solo.md#supported-devices) for inherited boards.

## Current tracking-enabled configurations

| Board | Environment | Adapter details |
| --- | --- | --- |
| LilyGO T-Echo Lite + KeyShield | `LilyGo_T-Echo-Lite_keyshield_zygis` | ES8311 speaker, existing TCA8418 keypad driver, status LEDs |
| ThinkNode M5 | `ThinkNode_M5_zygis` | Built-in buzzer and board-specific GPS power controls |
| Seeed Wio Tracker L1 / L1 Pro (OLED) | `WioTrackerL1_zygis` | Solo OLED display/joystick, onboard GPS and buzzer, dual BLE/USB |
| Seeed Wio Tracker L1 e-ink | `WioTrackerL1Eink_zygis` | Solo e-ink display/joystick, onboard GPS and buzzer, dual BLE/USB |

These environments have successful build checks. Physical GPS, controls, sound,
LEDs, persistence and power use still need validation. Other Solo environments
are retained; enabling tracking on additional boards needs integration and checks.

## T-Echo Lite LEDs and audio

Blue blinks for five seconds after a position-bearing status packet finishes
transmitting. Queueing/failed TX and unrelated radio traffic do not trigger it.
Green blinks for five seconds when GPS acquires/reacquires a fix, including after
a 15-second gap. Routine fix updates do not extend a burst. Both use 250ms on/off
without blocking. Normal Bluetooth and generic TX blinking are disabled.

The kit speaker uses the ES8311 codec and I2S pin mapping from
[LilyGo's kit example](https://github.com/Xinyuan-LilyGO/T-Echo-Lite/blob/main/examples/T-Echo-Lite-KeyShield/es8311/es8311.ino).
Solo's existing `TEchoKeyShield.cpp` owns keypad initialization and events; there
is no second keyboard driver consuming its FIFO. The pinned `cpp_bus_driver`
library builds only the selected codec and nRF52 bus support;
`scripts/techo_audio_build.py` excludes unrelated device drivers. Audio buffers
are serviced without waiting in the main loop. The DAC is powered off when idle.
Alarm dismissal consumes the button press so it does not also navigate/power off.

## ThinkNode M5 notes

Keep its physical GPS switch enabled; software cannot override a power cut.
The tracking environment uses `min_spiffs.csv` with two 1.875 MiB application
slots on a 4 MiB device. Back up data and use a full USB flash when changing
partition layouts; application-only OTA does not update the partition table.

## Building and flashing

Use your exact environment name, for example:

```sh
pio run -e LilyGo_T-Echo-Lite_keyshield_zygis
pio run -e ThinkNode_M5_zygis
```

Append `-t upload --upload-port YOUR_PORT` only after identifying the correct
port and reviewing the board's flashing/storage requirements. Port names depend
on the connected device and operating system.

## Wio Tracker L1 / L1 Pro OLED notes

`WioTrackerL1_zygis` extends Solo's OLED dual-interface target and enables the
shared tracking implementation, including Settings → Tracking. It retains the
existing OLED, joystick, GPS and buzzer drivers. Build with
`pio run -e WioTrackerL1_zygis`. A UF2 image is generated under
`.pio/build/WioTrackerL1_zygis/`; double-press RST to expose the bootloader drive
if automatic USB DFU entry is unavailable.

The optional `WioTrackerL1_zygis_games` target adds CPU/multiplayer Tic-Tac-Toe
and on-demand UI allocation. See [Games](games.md) for controls, private-channel
configuration, quick launch and the current physical-test limits.

## Wio Tracker L1 e-ink notes

`WioTrackerL1Eink_zygis` extends the existing e-ink Solo dual-interface target
and enables the shared tracking implementation. It retains the board's existing
GPS provider, display, joystick and buzzer. USB flashing uses the configured
nRF DFU uploader; a UF2 artifact is also generated.

A successful build or firmware upload alone does not establish GPS, radio,
audio or battery behavior. Record those checks separately.

## Adding another board

Enable `PRIVATE_STATUS_REPORTER` in an appropriate Solo target and implement the
GPS power/fresh-fix contract and available display/audio/control adapters.
Reuse existing board drivers, keep the shared tracking logic independent of pin
assignments, and document optional capabilities here. Add firmware coverage to
CI and record physical validation before claiming board support.

## Audio dependency licensing

The pinned ES8311 dependency `cpp_bus_driver` declares AGPL-3.0. Preserve its
notices and review distribution requirements for builds that include it.
