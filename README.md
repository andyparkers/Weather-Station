# Weather station project based on Arduino Nano and MH-Z19B
This is a simple weather station which can provide you with latest data about temperature, moisture, atmospheric pressure and CO2 level. Keeping the last one at optimal rate is essintial for being productive during the day, because exceeding some rate may make you feel tired and reduce your cognitive abilities.
# Wiring
Wiring is pretty simple. BME-280 and 2004 display are connected to default I2C pins - SCL to A5 and SDA to A4 - , MH-Z19B is connected to virtual UART interface at pins (0, 1). Sensor button output is connected to pin 5, can be changed as a constant value at source code.
