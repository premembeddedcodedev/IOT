BOARD?=esp32:esp32:esp32wrover:PartitionScheme=huge_app

PORT?=/dev/ttyUSB0

#TODO: Need to see why .PHONY required here....

.PHONY: default compile

default: compile
	@echo "===========In Default build============="

#all: req-libs-header lib-install compile flash clean

req-libs-header:
	# '@' sign before echo is not visible in print - print will be included echo if we give direct echo " "
	@echo "===== Dependent files as per header ======"
	# Calling like "echo" below also dont print "echo" command - its same as above
	echo Dependent files as per header
	arduino-manifest.pl  -r -b $(BOARD) *.ino *.h

lib-install:
	arduino-cli lib list
	arduino-cli lib install --git-url https://github.com/PaulStoffregen/RadioHead.git
	#arduino-cli lib install --git-url https://github.com/bblanchon/ArduinoJson.git
	arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncTCP.git
	arduino-cli lib install --git-url https://github.com/arkhipenko/TaskScheduler.git

lint:
	cpplint --extentions=ino --filter=-legal/copyright *.ino

compile:
	arduino-cli compile --fqbn $(BOARD) ./

flash:
	arduino-cli upload --fqbn $(BOARD) -p $(PORT) ./

clean:
	rm -f .*.hex
	rm -f .*.elf
