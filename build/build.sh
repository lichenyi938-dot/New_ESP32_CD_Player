#!/bin/bash
# Example build script for ESP-IDF (run inside a shell where idf is sourced)
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
if [ $? -eq 0 ]; then
  echo "Build succeeded. Firmware in build/ directory."
else
  echo "Build failed. See errors above."
  exit 1
fi
