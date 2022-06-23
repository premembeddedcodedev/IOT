BOARD?=esp8266:esp8266:nodemcuv2
PORT?=/dev/ttyUSB0
.PHONY: default all flash clean

default: all flash clean

lint:
	cpplint --extentions=ino --filter=-legal/copyright *.ino

all:
	arduino-cli compile --fqbn $(BOARD) ./

flash:
	arduino-cli upload --fqbn $(BOARD) -p $(PORT) ./

clean:
	rm -f .*.hex
	rm -f .*.elf
