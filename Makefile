.PHONY: all clean build fresh upload

all: fresh upload

clean:
	rm -rf build

build: build/dfr.ino.bin

fresh: clean build

upload: build
	esptool.py --port /dev/ttyUSB0 --baud 115200 write_flash --flash_size detect 0x00000 build/dfr.ino.bin

build/dfr.ino.bin: dfr/dfr.ino
	arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 --output-dir build dfr


