# ESP32-I2S-Internet-Radio
This project is an optimized Internet Radio streamer designed to demonstrate efficient embedded system architecture.
# ESP32 Internet Radio (I2S High-Fidelity Architecture)

A high-performance Internet Radio streamer built on the ESP32 platform. This project demonstrates advanced embedded system design by utilizing **Software MP3 Decoding** and a fully **Digital I2S Audio Path**, eliminating the noise and cost associated with traditional analog decoders.

## 🚀 Key Engineering Features
- **I2S Digital Audio:** Utilizes the MAX98357A Class-D amplifier for 24-bit audio resolution, maintaining signal integrity from the CPU to the speaker.
- **On-Board Software Decoding:** Leverages the ESP32's dual-core architecture to decode MP3/AAC streams in real-time, reducing the Bill of Materials (BOM).
- **WiFi Captive Portal:** Integrated web server for dynamic WiFi credential management. No hardcoding of passwords required.
- **Persistent Storage:** Uses the ESP32 `Preferences` library (NVS) to save volume and network settings permanently.
- **Robust Fail-Safe:** Implemented a watchdog logic to automatically reconnect and buffer streams during network instability.

## 🛠 Hardware Architecture

| Component | Specification | Function |
| :--- | :--- | :--- |
| **MCU** | ESP32 (30-pin DevKit) | Central Processing & WiFi |
| **Audio Amp** | MAX98357A | I2S Digital-to-Analog + Amp |
| **Display** | SH1106 OLED (I2C) | UI & Stream Metadata |
| **Regulator** | LM2596 Buck Module | Efficient 5V Power Delivery |
| **Inputs** | 4x Momentary Buttons | Volume & Channel Control |

## 🏗 System Design Decisions
- **Why I2S over DAC?** The ESP32's internal DAC is 8-bit and prone to noise. I2S keeps the signal digital, providing a high Signal-to-Noise Ratio (SNR).
- **Why LM2596?** To handle the high-current transients caused by the Wi-Fi radio and Class-D amplifier spikes that linear regulators cannot handle efficiently.
- **Non-Blocking Logic:** The firmware is designed without `delay()` calls, ensuring the audio buffer is constantly fed while the UI remains responsive.

## 📂 Repository Structure
- `/Firmware`: Source code (.ino) and library requirements.
- `/Hardware`: Schematic diagrams and pinout definitions.
- `/Media`: Photos and demo videos of the working prototype.

## 🔧 Setup & Installation
1. **Hardware:** Wire the components according to the schematic in the `/Hardware` folder.
2. **Library:** Install the `ESP32-audioI2S` library via Arduino Library Manager.
3. **Flash:** Upload the code to your ESP32.
4. **Configure:** 
   - On first boot, connect to the WiFi network: `ESP32_Radio_Setup`.
   - Open `192.168.4.1` in your browser.
   - Enter your WiFi credentials. The device will restart and begin streaming.

## 📜 License
This project is licensed under the **MIT License** - see the LICENSE file for details.

## 🤝 Acknowledgments
Inspired by the *educ8s* Internet Radio project, with significant upgrades to the audio architecture and network management system.

