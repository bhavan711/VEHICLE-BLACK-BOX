# Vehicle Black Box

An embedded vehicle monitoring and data-recording system for crash detection, event logging, and remote monitoring — built on ESP32.

## Overview

The Vehicle Black Box continuously monitors vehicle conditions in real time, detects abnormal events such as crashes or sudden impacts, logs critical data locally, and supports wireless remote monitoring — functioning much like a flight data recorder, but for road vehicles.

## Architecture

```
                 Vehicle
                    │
                    ▼
              ┌───────────┐
              │   ESP32   │
              │ Controller│
              └─────┬─────┘
                    │
          ┌─────────┴─────────┐
          ▼                   ▼
   ┌──────────────┐    ┌──────────────┐
   │ Local Data   │    │ Wireless /   │
   │ Logging      │    │ IoT Platform │
   └──────┬───────┘    └──────────────┘
          │
          ▼
   ┌──────────────┐
   │ Event and    │
   │ Status Data  │
   └──────────────┘
```

## Features

- Crash and impact detection
- Vehicle location tracking
- Local data logging
- Wireless communication
- Real-time status monitoring
- Event recording
- IoT-based remote monitoring
- Continuous vehicle monitoring

## How It Works

1. The ESP32 continuously monitors the vehicle during operation
2. Collected data is processed in real time
3. Abnormal events (sudden impacts, crashes) are detected
4. Event data is stored locally for later analysis
5. Vehicle location is recorded when required
6. Event data is transmitted for remote monitoring
7. Real-time vehicle status is made available throughout

## Hardware Used

| Component | Purpose |
|---|---|
| ESP32 | Main controller — sensing, processing, logging, wireless comms |
| IMU / accelerometer | Crash and impact detection |
| GPS module | Vehicle location tracking |
| SD card / flash storage | Local event and status data logging |
| WiFi / IoT module | Remote monitoring and data transmission |

## Repository Structure

```
Vehicle-Black-Box/
├── src/
│   └── blackbox.ino
├── data/
│   └── sample_data/
├── docs/
│   └── project_documentation.pdf
├── images/
│   └── project_images/
└── README.md
```

## Applications

- Vehicle accident analysis
- Vehicle safety monitoring
- Event recording
- Fleet monitoring
- Automotive IoT applications
- Embedded systems research
- Academic projects

## Future Improvements

- Advanced accident classification
- Mobile application integration
- Cloud-based historical data analysis
- Improved event detection algorithms
- Automated emergency notifications
- Web-based monitoring dashboard

## Status

Educational / research prototype.

## License

All Rights Reserved — see LICENSE for details.
