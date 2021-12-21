# IOT
IOT project

Pre-requisites for  (arduino-cli) Arduino Command Line Interface in ubuntu:

      1. Download arduino-cli utility from internet, we have two options for arduino availability
          a. directly download utility from internet - https://arduino.github.io/arduino-cli/0.20/installation/ (or) https://github.com/arduino/arduino-cli/releases
          b. or download source code and extract and compile it - https://github.com/arduino/arduino-cli
      
      2. once we get utility from above resources, we need to copy arduino-utility into /usr/local/bin/arduino-cli
      
      3. Add platform, boards, upload and compilation procedure to arduino-cli.Reference: https://create.arduino.cc/projecthub/B45i/getting-started-with-arduino-cli-7652a5
      
      4. PLatform and board installation
          a. Install Support Arduino Board cores
          b. Install Support for 3rd party Board cores
      
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
            a. arduino-cli lib search LibraryName (Reference: https://www.youtube.com/watch?v=cVod8k713_8&t=604s)
            b. praveenv@praveenv-vm:~/Desktop$ arduino-cli lib list
                  Name    Installed     Available         Location Description
                  ssd1306 1.8.3         -                 user     -

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
      
      14. Reference: https://www.devdungeon.com/content/arduino-cli-tutorial
