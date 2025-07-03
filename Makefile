FQBN = esp32:esp32:esp32c3
PORT = /dev/cu.usbmodem101
BAUD = 115200
FLASH_SIZE = 4MB
CHIP = esp32c3

.PHONY: all clean build upload monitor debug

all: clean build upload

debug: clean build upload monitor

clean:
	rm -rf build

build: build/dfr.ino.bin

upload:
	esptool.py --chip $(CHIP) --port $(PORT) --baud $(BAUD) --before default_reset --after hard_reset \
		write_flash --flash_mode dio --flash_size $(FLASH_SIZE) --flash_freq 40m 0x0 build/dfr.ino.bootloader.bin 0x8000 build/dfr.ino.partitions.bin 0x10000 build/dfr.ino.bin

monitor:
	screen $(PORT) $(BAUD)

build/dfr.ino.bin: dfr/dfr.ino
	arduino-cli compile --fqbn $(FQBN) --output-dir build dfr
