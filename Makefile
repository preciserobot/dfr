FQBN = esp32:esp32:esp32c3
#FQBN = esp8266:esp8266:d1_mini
#FQBN = esp8266:esp8266:nodemcuv2

PORT = /dev/cu.usbmodem101
#PORT = /dev/ttyUSB0

BAUD = 115200

FLASH_SIZE = 4MB
#FLASH_SIZE = detect

CHIP = esp32c3
#CHIP = esp8266


.PHONY: all clean build fresh upload

all: fresh upload

clean:
	rm -rf build

build: build/dfr.ino.bin

fresh: clean build

upload: build
	#esptool [-h] [--chip {auto,esp8266,esp32,esp32s2,esp32s3beta2,esp32s3,esp32c3,esp32c6beta,esp32h2beta1,esp32h2beta2,esp32c2,esp32c6,esp32c61,esp32c5,esp32c5beta3,esp32h2,esp32p4}] [--port PORT]
    #           [--baud BAUD] [--port-filter PORT_FILTER] [--before {default_reset,usb_reset,no_reset,no_reset_no_sync}] [--after {hard_reset,soft_reset,no_reset,no_reset_stub}] [--no-stub] [--trace]
    #           [--override-vddsdio [{1.8V,1.9V,OFF}]] [--connect-attempts CONNECT_ATTEMPTS]
    #           {write_flash,
	esptool.py --chip $(CHIP) --port $(PORT) --baud $(BAUD) --before default_reset --after hard_reset \
		write_flash --flash_mode dio --flash_size $(FLASH_SIZE) --flash_freq 40m 0x0 build/dfr.ino.bootloader.bin 0x8000 build/dfr.ino.partitions.bin 0x10000 build/dfr.ino.bin


build/dfr.ino.bin: dfr/dfr.ino
	arduino-cli compile --fqbn $(FQBN) --output-dir build dfr

monitor:
	pyserial-miniterm $(PORT) $(BAUD)

