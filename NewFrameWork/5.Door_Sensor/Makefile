BOARD?=esp8266:esp8266:nodemcuv2
PORT?=/dev/ttyUSB0
.PHONY: default all flash clean

default: lib-install all flash clean

lib-install:
	arduino-cli lib list
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
