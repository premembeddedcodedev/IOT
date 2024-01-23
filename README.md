# IOT

Arduino Commandline interface (arduino-cli):

Note: if you want to change the path for downloading the packages, please copy entire ~/.arduino15 folder into your path and change the directory in ~/.arduino15 default folder.

export TMPDIR=/mnt/arduino-install/temp

yaml (~/.arduino15/arduino-cli.yaml) file is a configuration file which we need to change whenever we want to install any library and board configurations

      Pre-requisites for  (arduino-cli) Arduino Command Line Interface in ubuntu:

      1. Download arduino-cli utility from internet, we have two options for arduino availability
          a. directly download utility from internet - https://arduino.github.io/arduino-cli/0.20/installation/ (or) https://github.com/arduino/arduino-cli/releases
          b. or download source code and extract and compile it - https://github.com/arduino/arduino-cli
      
      2. once we get utility from above resources, we need to copy arduino-utility into /usr/local/bin/arduino-cli
      
      3. Add platform, boards, upload and compilation procedure to arduino-cli.Reference: https://create.arduino.cc/projecthub/B45i/getting-started-with-arduino-cli-7652a5
      
      4. PLatform and board installation
          a. Install Support Arduino Board cores
                  1. arduino-cli core install arduino:avr
          b. Install Support for 3rd party Board cores
          c. arduino-cli board listall | grep arduino | grep uno
      
      5. arduino-cli config init -- this can be done once 
      
      6. Add the board details in arduino-cli.yaml and updare arduino/3rd party board details using command: "arduino-cli core update-index"
            board_manager:
                  additional_urls:
      
      7. Arduino/3rd party board installations using yaml scripts: arduino-cli core search uno
                  Ex: praveenv@praveenv-vm:~/Desktop$ arduino-cli core search uno
                  Updating index: package_index.json downloaded
                  Updating index: package_index.json.sig downloaded
                  Updating index: package_esp8266com_index.json downloaded
                  Updating index: package_esp32_index.json downloaded
                  Updating index: package_esp32_index.json downloaded
                  Updating index: package_rp2040_index.json downloaded
                  ID              Version Name
                  arduino:avr     1.8.4   Arduino AVR Boards
                  arduino:megaavr 1.8.7   Arduino megaAVR Boards
      
      8. arduino-cli core list - it will display the core list on connected board.
                  Ex: praveenv@praveenv-vm:~/Desktop/sketch2$ arduino-cli core list
                        ID              Installed Latest Name
                        esp8266:esp8266 3.0.2     3.0.2  esp8266
      
      9. Installations:
            a. Core Installation: arduino-cli core install esp8266:esp8266 
      
      9. Library installation: 
            a. arduino-cli lib search LibraryName
            b. arduino-cli lib install <LibraryName>
            c. praveenv@praveenv-vm:~/Desktop$ arduino-cli lib list
                  Name    Installed     Available         Location Description
                  ssd1306 1.8.3         -                 user     -
                  
            c. arduino-cli lib update-index
            d. arduino-cli lib uninstall "ssd1306"
            e. arduino-cli lib install AudioZero       # for the latest version.
               arduino-cli lib install AudioZero@1.0.0 # for the specific version.
               arduino-cli lib install --git-url https://github.com/arduino-libraries/WiFi101.git https://github.com/arduino-libraries/ArduinoBLE.git
               arduino-cli lib install --zip-path /path/to/WiFi101.zip /path/to/ArduinoBLE.zip

            References: https://arduino.github.io/arduino-cli/0.20/commands/arduino-cli_lib_install/

      10. Few arduino commands: 
            a.arduino-cli board list - it will display the connected board details
                  Ex:praveenv@praveenv-vm:~/Desktop/sketch2$ arduino-cli board list
                  Port         Type              Board Name FQBN Core
                  /dev/ttyS9   Serial Port       Unknown
                  /dev/ttyUSB0 Serial Port (USB) Unknown

            b. arduino-cli board listall - it will display all the supported boards list upon adding yaml additional URLs
                  praveenv@praveenv-vm:~/Desktop$ arduino-cli board listall
                  Board Name                      FQBN
                  DOIT ESP32 DEVKIT V1            esp32:esp32:esp32doit-devkit-v1
                  NodeMCU 1.0 (ESP-12E Module)    esp8266:esp8266:nodemcuv2

      7. Create new sketch
            arduino-cli sketch new Sample
            
      8. compile the sketch using
            arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 sketch
            (or) arduino-cli compile -b esp8266:esp8266:nodemcuv2 sketch
            
      9. upload using 
            arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcuv2 sketch
 
      10. Sample /home/praveenv/.arduino15/arduino-cli.yaml file for installing boards and platforms

            Sample code:
            -----------

            board_manager:
              additional_urls: [https://arduino.esp8266.com/stable/package_esp8266com_index.json, https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json]
            daemon:
              port: "50051"
            directories:
              data: /home/praveenv/.arduino15
              downloads: /home/praveenv/.arduino15/staging
              user: /home/praveenv/Arduino
            library:
              enable_unsafe_install: false
            logging:
              file: ""
              format: text
              level: info
            metrics:
              addr: :9090
              enabled: true
            sketch:
              always_export_binaries: false
      
      12. arduino-cli config dump --verbose

      13. install upon updating yaml file: arduino-cli core install esp32:esp32
      
      14. Reference: 
      
      15. arduino-cli board details esp8266:esp8266:nodemcuv2
            praveenv@praveenv-vm:~/Desktop/sketch2$ arduino-cli board details esp8266:esp8266:nodemcuv2
            Board name:            NodeMCU 1.0 (ESP-12E Module)
            FQBN:                  esp8266:esp8266:nodemcuv2
            Board version:         3.0.2

            Package name:          esp8266
            Package maintainer:    ESP8266 Community
            Package URL:           https://arduino.esp8266.com/stable/package_esp8266com_index.json
            Package website:       https://github.com/esp8266/Arduino
            Package online help:   https://esp8266.com/arduino

            Platform name:         esp8266
            Platform category:     ESP8266
            Platform architecture: esp8266
            Platform URL:          https://github.com/esp8266/Arduino/releases/download/3.0.2/esp8266-3.0.2.zip
            Platform file name:    esp8266-3.0.2.zip
            Platform size (bytes): 38204730
            Platform checksum:     SHA-256:653add97df0a850ebc26f105355be749fb83c2db7b1ddf3e77950e694059eb9c

            Required tool:         esp8266:xtensa-lx106-elf-gcc                                                   3.0.4-gcc10.3-1757bed

            Required tool:         esp8266:mkspiffs                                                               3.0.4-gcc10.3-1757bed

            Required tool:         esp8266:mklittlefs                                                             3.0.4-gcc10.3-1757bed

            Required tool:         esp8266:python3                                                                3.7.2-post1

            Option:                CPU Frequency                                                                  xtal
                                   80 MHz                                                                    ✔    xtal=80
                                   160 MHz                                                                        xtal=160
            Option:                VTables                                                                        vt
                                   Flash                                                                     ✔    vt=flash
                                   Heap                                                                           vt=heap
                                   IRAM                                                                           vt=iram

      16. arduino-cli sketch new <sample_prog_name>
      17. [NewFrameWork/ArdunioMakefile for the same]Other References for compilation: https://opensourcelibs.com/lib/esp8266-arduino-makefile
            a. sudo apt-get update
            b. sudo apt-get install arduino-mk
            c. Link library ln -s /usr/share/arduino/Arduino.mk
            d. create example folder and .ino file then create : mkdir libraries
            e. make
            f. cd build-uno/
            g. make upload
            h. make upload clean
            i. sudo apt-get install screen
            j. make upload monitor clean
            k. screen -list
            l. screen -r
            m. screen -X quit
            n. Makefile Example:
                  ARDUINO_DIR = /usr/share/arduino
                  ARDUINO_PORT = /dev/ttyUSB0
                  USER_LIB_PATH = /home/praveenv/sambashare/NodeMCUProjects/sample/libraries
                  BOARD_TAG = esp8266com:esp8266:generic //TODO: Not working -- <Need to decide based on board type - unfortunately dint work>
                  include /usr/share/arduino/Arduino.mk

      18. SDK https://github.com/esp8266/esp8266-wiki/wiki/Toolchain
      
 ESP32 CAM: (Board Settings: /home/praveenv/.arduino15/packages/esp32/hardware/esp32/2.0.2/boards.txt - to select app storage esp32wrover.menu.PartitionScheme.huge_app=Huge APP (3MB No OTA/1MB SPIFFS))
 
            1. Connect 3.3v pins at the time of flashing
            2. Connect IO0 and Gnd short for flashing
            3. connect Tx->RX amd Rx->Tx for UART
            4. We need to connect 5v for camera to work
            5. settings for 
                  a. selecting right board: ESP32 Wroover module
                  b. choose 115200 as baud rate
                  c. select huge app 8MB
                  d. select correct COM port
                  e. choose AI THINKER for camera module to work
                  
            6. Select Camserver for example from file->example
