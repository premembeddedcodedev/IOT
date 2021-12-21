# IOT
IOT project

Pre-requisites for  (arduino-cli) Arduino Command Line Interface in ubuntu:

      1. Download arduino-cli utility from internet, we have two options for arduino availability
          a. directly download utility from internet - https://arduino.github.io/arduino-cli/0.20/installation/
          b. or download source code and extract and compile it - https://github.com/arduino/arduino-cli
      
      2. once we get utility from above resources, we need to copy arduino-utility into /usr/local/bin/arduino-cli
      
      3. Add platform, boards, upload and compilation procedure to arduino-cli.Reference: https://create.arduino.cc/projecthub/B45i/getting-started-with-arduino-cli-7652a5
      
      4. PLatform and board installation
          a. Install Support Arduino Board cores
          b. Install Support for 3rd party Board cores
      
      5. Installations:
            a. Library installation: arduino-cli lib search LibraryName
            b. Core Installation: arduino-cli core install esp8266:esp8266
      
      6. arduino-cli config init -- this can be done once
      
      7. Add the board details in arduino-cli.yaml and updare arduino/3rd party board details using command: "arduino-cli core update-index"
            board_manager:
                  additional_urls:
      
      6. Few arduino commands:
            a.arduino-cli board list - it will display the connected board details
                  Ex:praveenv@praveenv-vm:~/Desktop/sketch2$ arduino-cli board list
                  Port         Type              Board Name FQBN Core
                  /dev/ttyS9   Serial Port       Unknown
                  /dev/ttyUSB0 Serial Port (USB) Unknown

            b. arduino-cli core list - it will display the core list on connected board.
                  Ex: praveenv@praveenv-vm:~/Desktop/sketch2$ arduino-cli core list
                        ID              Installed Latest Name
                        esp8266:esp8266 3.0.2     3.0.2  esp8266
                        
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
