#BOARD?=arduino:avr:nano
BOARD?=arduino:avr:nano:cpu=atmega328old
PORT?=/dev/ttyUSB0

.PHONY: default compile

default: compile

all: req-libs-header lib-install compile flash clean

req-libs-header:
	arduino-manifest.pl  -r -b $(BOARD) *.ino *.h

lib-install:
	arduino-cli lib list
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BusIO.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_BME280_Library.git 
	arduino-cli lib install --git-url https://github.com/adafruit/Adafruit_Sensor.git 
	arduino-cli lib install --git-url https://github.com/PaulStoffregen/RadioHead.git
	#arduino-cli lib install --git-url https://github.com/bblanchon/ArduinoJson.git
	arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncTCP.git
	arduino-cli lib install --git-url https://github.com/arkhipenko/TaskScheduler.git

lint:
	cpplint --extentions=ino --filter=-legal/copyright *.ino

compile:
	arduino-cli compile --fqbn $(BOARD) ./ --output-dir ./out

flash:
	arduino-cli upload --fqbn $(BOARD) -p $(PORT) ./

clean:
	rm -rf out
