BOARD?=esp8266:esp8266:nodemcuv2

PORT?=/dev/ttyUSB0

#TODO: Need to see why .PHONY required here....

.PHONY: default compile

default: clean compile
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
	arduino-cli lib install --git-url https://github.com/martin-ger/uMQTTBroker.git
	arduino-cli lib install "PubSubClient"
	arduino-cli lib install "AsyncElegantOTA"
	#arduino-cli lib install --git-url https://github.com/bblanchon/ArduinoJson.git
	arduino-cli lib install --git-url https://github.com/me-no-dev/ESPAsyncTCP.git
	arduino-cli lib install --git-url https://github.com/arkhipenko/TaskScheduler.git

lint:
	cpplint --extentions=ino --filter=-legal/copyright *.ino

compile:
	#arduino-cli compile --fqbn $(BOARD) ./ --output-dir ./out --show-properties
	arduino-cli compile --fqbn $(BOARD) ./ --output-dir ./out
	$(shell) ls ./out/
flash:
	#arduino-cli upload --fqbn $(BOARD) -p $(PORT) ./
	arduino-cli upload --fqbn $(BOARD) -p $(PORT) --input-dir ./out/

clean:
	rm -rf out

ota:
	python ./espota.py -i 192.168.29.65 -f ./out/2.MasterNode_consumer.ino.bin
	rm -rf out

ota_old:
	#python ../tools/espota.py -i 192.168.29.119 -f ./out/OTA.ino.bin
	#@PLAT_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.platform.path' | awk -F= '{print $$2}'` ; \
	   PY_PATH=`arduino-cli compile -b $(FQBN) --show-properties | grep '^runtime.tools.python3.path' | awk -F= '{print $$2}'` ; \
	IOT_IP=`avahi-browse _arduino._tcp --resolve --parsable --terminate|grep -i ';$(IOT_NAME);'|grep ';$(OTA_PORT);'| awk -F\; '{print $$8}'|head -1`; \
	BINFILE=$(wildcard $(BUILD_DIR)/$(SRC)*bin); \
	echo "PLAT_PATH  is [$$PLAT_PATH]" ; \
	echo "PY_PATH:   is [$$PY_PATH]"  ; \
	echo "IOT_IP:    is [$$IOT_IP]"   ; \
	echo "BINFILE:   is [$$BINFILE]"  ; \
	if [ "$$IOT_IP" = "" ] ; \
	then echo "Unable to find device IP. Check that the IOT_NAME environment variable is correctly set. Use 'make find' to search devices"; \
	else echo "---> Uploading Over The Air"; \
	$$PY_PATH/python3 $$PLAT_PATH/tools/espota.py -i $$IOT_IP -p $(OTA_PORT) --auth=$(OTA_PASS) -f $$BINFILE ;\
	fi
