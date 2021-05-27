# IOTA ESP32 Wallet  

IOTA wallet application for Chrysalis network.

## Requirements  

* [ESP32-DevKitC V4](https://docs.espressif.com/projects/esp-idf/en/latest/hw-reference/get-started-devkitc.html#functional-description)


## Build system setup

Please follow documentations to setup your toolchain and development framework.

* [esp32 get started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html)
* [esp32-c3 get started](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)


**Notice: We use the master branch of ESP-IDF, make sure you clone the right branch of ESP-IDF**

Update esp-idf from previous version
```
$ git clone --recursive https://github.com/espressif/esp-idf.git
$ cd esp-idf
$ git submodule update --init --recursive
$ ./install.sh
$ source ./export.sh
```

### Building wallet

```
$ git clone https://github.com/oopsmonk/iota_esp32_wallet.git
$ cd iota_esp32_wallet
$ git checkout dev_chrysalis
$ git submodule update --init --recursive
```
### ESP32 target

```
$ idf.py menuconfig
$ idf.py build
```

### ESP32-C3 target

```
$ idf.py set-target esp32c3
$ idf.py menuconfig
$ idf.py build
```

## Wallet configuration

### Flash wallet application

## Troubleshooting

`E (38) boot_comm: This chip is revision 2 but the application is configured for minimum revision 3. Can't run.`
I'm using ESP32-C3 Rev2 but the current ESP-IDF uses Rev 3 as default, we need to change it via `idf.py menuconfig`
```
Component config ---> ESP32C3-Specific ---> Minimum Supported ESP32-C3 Revision (Rev 3)  ---> Rev 2
```
