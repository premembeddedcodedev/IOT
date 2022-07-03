BOARD?=arduino:avr:nano
PORT?=/dev/ttyUSB0
.PHONY: default all flash clean

default: lib-install all flash clean

lib-install:
	arduino-cli lib list
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_SSD1306.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit-GFX-Library.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BusIO.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BME280_Library.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_Sensor.git 
	arduino-cli lib install --git-url https://github.com/PaulStoffregen/RadioHead.git
	#arduino-cli lib install --git-url https://github.com/bblanchon/ArduinoJson.git
	arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncTCP.git
	arduino-cli lib install --git-url https://github.com/arkhipenko/TaskScheduler.git

lint:
	cpplint --extentions=ino --filter=-legal/copyright *.ino

all:
	arduino-cli compile --fqbn $(BOARD) ./

flash:
	arduino-cli upload --fqbn $(BOARD) -p $(PORT) ./

clean:
	rm -f .*.hex
	rm -f .*.elf
