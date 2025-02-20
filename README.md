# KepecsWheel Library

An ESP32-based Arduino library for monitoring and logging mouse wheel rotations in neuroscience experiments.

## Updating Firmware

1. Download the KepecsWheel library for the Arduino IDE or manually clone/download the repository from [Neurotech-Hub/KepecsWheel](https://github.com/Neurotech-Hub/KepecsWheel). For downloaded libraries, go to Sketch -> Include Library -> Add .ZIP Library, or place the library in the `libraries` folder in the Arduino IDE.
2. If the device has been previously flashed, it may be in a sleep state that prevents it from connecting to the serial port. To enter boot mode, hold the `Boot` button and toggle the `Reset` button (then release the `Boot` button).
3. If the Arduino IDE does not indicate that it is connected to "Adafruit Feather ESP32-S3 2MB PSRAM", click Tools -> Board -> esp32 -> Adafruit Feather ESP32-S3 2MB PSRAM (you will need to download the esp32 board package (by espressif) from the Arduino IDE).

If you wish to modify the sketch itself, it is highly recommended to become a collaborator on this repository and use proper git workflows. This will reduce the risk of delpoying out of date code and makes identifying errors easier across multilpe authors.

## Hublink

[Hublink.cloud](https://hublink.cloud) is meant to transfer SD card content to the cloud. Your lab will have a dashboard link that should be for internal use only. The [Hublink Docs](https://hublink.cloud/docs) contain information about how format the `meta.json` file on the SD card, which is critical for:

1. Setting up a path structure for the cloud storage.
2. Setting subject data that can be used to track the experiments.
3. Adjust settings of the Hublink sync process.

### meta.json

```json
{
  "hublink": {
    "advertise": "HUBLINK",
    "try_reconnect": false,
    "upload_path": "/WHEEL",
    "append_path": "subject:id/experimenter:name",
    "disable": false
  },
  "wheel": {
    "sleep_time_seconds": 60,
    "sync_every_minutes": 10,
    "sync_for_seconds": 30
  },
  "subject": {
    "id": "mouse001",
    "strain": "C57BL/6",
    "sex": "male",
  },
  "experimenter": {
    "name": "john_doe"
  }
}
``

### RTC

When connecting to Hublink, the RTC will be set to the current time via the `onTimestampReceived` callback.

## License

This project is licensed under the MIT License. 