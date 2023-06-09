# led-squares-example
[![Test](https://github.com/Johboh/led-squares-example/actions/workflows/test.yaml/badge.svg)](https://github.com/Johboh/led-squares-example/actions/workflows/test.yaml)

An example project for a LED matrix, as described on [Printables](https://www.printables.com/model/472278-modular-led-panelmatrix).

## Prerequisites
- [A LED panel](https://www.printables.com/model/472278-modular-led-panelmatrix)
- [Visual Studio Code](https://code.visualstudio.com/)
- [Platform I/O extension](https://platformio.org/install/ide?install=vscode)
- In this example, a [ESP32-S2](https://www.wemos.cc/en/latest/s2/s2_mini.html)
- [imagemagick](https://imagemagick.org/)

## Getting started
- Set your WiFi credentials in [credentials.h](./src/credentials.h).
- [platform.ini](./src/platform.ini), adjust `upload_port` to your device upload port.
- Adjust the `NUM_COLUMNS` and `NUM_ROWS` in [main.cpp](./src/main.cpp#L13) to match the size of your panel.
- Compile and upload the project.
- Adjust the IP number in [platform.ini](./src/platform.ini) to match the IP number of your device. the IP will be printed in the console on boot.
- Adjust the IP in [upload_gif.sh](./scripts/upload_gif.sh#L16-L17) and [upload_static.sh](./scripts/upload_static.sh#L15-L16) to match the IP of your device.
- Adjust the panel pixel width and height in [upload.py](./scripts/upload.py#L9), [upload_static.sh](./scripts/upload_static.sh#L14) and [upload_gif.sh](./scripts/upload_gif.sh#L14). You can search for `56` to find all occurances (corresponding to a panel width of 7 tiles, each tile 8 pixels wide).
- To upload a image, go to the [scripts](./scripts) folder and run `./upload_static.sh /path/to/image.png`

## Dependencies
- https://github.com/Johboh/ConnectionHelper @^1.0.8
- https://github.com/FastLED/FastLED @^3.5.0
- https://github.com/tobozo/ESP32-PSRamFS @^1.0.4-beta
- https://github.com/BertanT/Arduino-RGBWConverter.git
- Needs C++17 for `std::optional`.
  - For platform I/O in `platformio.ini`:
    ```C++
    build_unflags=-std=gnu++11 # "Disable" C++11
    build_flags=-std=gnu++17 # "Enable" C++17
    ```
