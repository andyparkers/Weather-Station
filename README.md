# Weather station project based on Arduino Nano and MH-Z19B
This is a simple weather station which can provide you with latest data about temperature, moisture, atmospheric pressure and CO2 level. Keeping the last one at optimal rate is essential for being productive during the day, because exceeding some rate may make you feel tired and reduce your cognitive abilities.
# Requirements
This is what you need to accomplish this project:
- 2004 I2C Display
- BME-280
- MH-Z19B
- Arduino Nano
- Sensor button
- USB cable
# Wiring
Wiring is pretty simple. BME-280 and 2004 display are connected to default I2C pins SCL to A5 and SDA to A4, MH-Z19B is connected to virtual UART interface at pins (0, 1). Sensor button output is connected to pin 5, this pin can be changed as a value of a constant variable at the source code:

> const int BUTTON_PIN = 5;

Everything is powered with USB cable, so you are supposed to get one and connect VCC and GND to suitable pins at Arduino board.
# How does it eventually look like?
Main screen

![Main screen](https://github.com/andyparkers/Weather-Station/blob/main/Images/IMG_20220729_201324_BURST1.jpg)

CO2 ppm graph

![ppm](https://github.com/andyparkers/Weather-Station/blob/main/Images/IMG_20220729_201343.jpg)

Temperature graph

![temperature](https://github.com/andyparkers/Weather-Station/blob/main/Images/IMG_20220729_201356.jpg)

Moisture graph

![moisture](https://github.com/andyparkers/Weather-Station/blob/main/Images/IMG_20220729_201409.jpg)

Atmospheric pressure graph

![atmospheric pressure](https://github.com/andyparkers/Weather-Station/blob/main/Images/IMG_20220729_201416.jpg)


