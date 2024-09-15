# Weather Station with ESP32 and Email Notification System

This project demonstrates a weather monitoring system using the ESP32 microcontroller, equipped with sensors to monitor environmental parameters such as temperature, humidity, pressure, and altitude. It also features an email notification system that sends alerts when the measured values exceed user-defined thresholds. Users can interact with the system via a web interface to set thresholds, enable/disable email notifications, and receive real-time data from the sensors.

## Features

- **Temperature and Humidity Monitoring**: Using the DHT11 sensor to measure temperature and humidity.
- **Pressure and Altitude Monitoring**: Using the BMP180 sensor to measure atmospheric pressure and altitude.
- **Email Alerts**: Configured to send email notifications when temperature, humidity, pressure, or altitude exceeds predefined thresholds.
- **Web Interface**: A simple HTML web interface hosted on the ESP32 where users can set thresholds, view real-time data, and manage email notifications.
- **ESP32 Email Sending**: Utilizes the ESP32 to send email notifications through Gmail's SMTP server.
- **Dynamic Web Updates**: The webpage updates dynamically with real-time sensor data.

## Components Used

- **ESP32**: Main microcontroller.
- **DHT11 Sensor**: Measures temperature and humidity.
- **BMP180 Sensor**: Measures atmospheric pressure and altitude.
- **Wi-Fi**: ESP32 connects to a Wi-Fi network to host the web server and send emails.

## Required Libraries

1. Wi-Fi
2. Adafruit Sensor Library
3. DHT Sensor Library
4. Adafruit BMP085 Unified
5. PubSubClient
6. ESPAsyncWebServer
7. ESP32 Mail Client


### Web Interface

- **View Real-time Data**: The homepage displays the current temperature, humidity, pressure, and altitude.
- **Configure Thresholds**: Enter custom thresholds for temperature, humidity, pressure, and altitude. If the measured value exceeds any threshold, an email notification is sent.
- **Enable/Disable Email Notifications**: Enable or disable email notifications by checking/unchecking the option in the web form.

## Email Notification

The system will send an alert email to the configured recipient when any of the sensor readings exceed the set thresholds. The email contains the current value of the sensor that triggered the alert.

## Customization

You can customize the project by:
- Adding more sensors (e.g., air quality sensors).
- Extending the web interface to include additional data.
- Using other email servers for notifications.


