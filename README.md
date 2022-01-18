# **This repo has been moved to [iotaledger/esp32-client-sdk](https://github.com/iotaledger/esp32-client-sdk)**

# IOTA ESP32 Wallet  

This is an IOTA Wallet application using [IOTA CClient](https://github.com/iotaledger/entangled/tree/develop/cclient) library on [ESP32](https://en.wikipedia.org/wiki/ESP32) microcontroller.  

## Commands  

#### System commands  
* `help`: Show help
* `version`: Show version info
* `restart`: Restart ESP32
* `free`: Show remained heap size
* `stack`: Show stack info
* `node_info`: Show IOTA node info
* `node_info_set`: Set IOTA node URL and port number

#### IOTA Client commands  
* `seed`: Show IOTA seed
* `seed_set`: Set IOTA seed
* `balance`: Get balance from given addresses
* `account`: Get balances from current seed
* `send`: Send valued or data transactions
* `transactions`: Get transactions from a given address
* `gen_hash`: Generate hash from a given length
* `get_addresses`: Generate addresses from given index.
* `get_bundle`: Get a bundle from a given transaction tail.
* `client_conf`: Show current MWM, Depth, and Security level
* `client_conf_set`: Set MWM, Depth, and Security level.

## Block Diagram  

![](https://raw.githubusercontent.com/oopsmonk/iota_esp32_wallet/master/images/esp32_wallet_block_diagram.png)

## Demonstration video 

[![](http://img.youtube.com/vi/e6pxDTqd5Pw/0.jpg)](http://www.youtube.com/watch?v=e6pxDTqd5Pw)  

## Requirements  

* [ESP32-DevKitC V4](https://docs.espressif.com/projects/esp-idf/en/latest/hw-reference/get-started-devkitc.html#functional-description)

## ESP32 build system setup  

Please follow documentations to setup your toolchain and development framework.

Linux:  
* [xtensa-esp32 toolchain](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started-cmake/linux-setup.html) 
* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started-cmake/index.html#get-esp-idf) 

Windows:
* [xtensa-esp32 toolchain](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started-cmake/windows-setup.html#standard-setup-of-toolchain-for-windows-cmake) 
* [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/v3.3.1/get-started-cmake/index.html#windows-command-prompt) 

**Notice: We use the ESP-IDF v4.0.1, make sure you clone the right branch of ESP-IDF**

```
git clone -b v4.0.1 --recursive https://github.com/espressif/esp-idf.git
./install.sh
source ./export.sh
```

Update esp-idf from previous version
```
cd esp-idf
git fetch 
git checkout v4.0.1 -b v4.0.1
git submodule update --init --recursive
./install.sh
source ./export.sh
```


Now, you can test your develop environment via the [hello_world](https://github.com/espressif/esp-idf/tree/release/v3.2/examples/get-started/hello_world) project.  

```shell
cd ~/esp
cp -r $IDF_PATH/examples/get-started/hello_world .
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor
```

The output would be something like:  

```shell
I (0) cpu_start: App cpu up.
I (184) heap_init: Initializing. RAM available for dynamic allo
cation:
I (191) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (197) heap_init: At 3FFB2EF8 len 0002D108 (180 KiB): DRAM
I (204) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (210) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (216) heap_init: At 40089560 len 00016AA0 (90 KiB): IRAM
I (223) cpu_start: Pro cpu start user code
I (241) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
Hello world!
This is ESP32 chip with 2 CPU cores, WiFi/BT/BLE, silicon revision 1, 4MB external flash
Restarting in 10 seconds...
Restarting in 9 seconds...
```

You can press `Ctrl` + `]` to exit the monitor and ready for the next setup.  

## Building and flashing Wallet application to ESP32

### Step 1: cloning wallet repository  

```shell
git clone --recursive https://github.com/oopsmonk/iota_esp32_wallet.git
```

Or (if you didn't put the `--recursive` command during clone)  

```shell
git clone https://github.com/oopsmonk/iota_esp32_wallet.git
cd iota_esp32_wallet
git submodule update --init --recursive
```

### Step 2: initializing components

The `init.sh` helps us to generate files and switch to the right branch for the components.  

Linux:

```shell
cd iota_esp32_wallet
bash ./init.sh
```

Windows: **TODO**  

### Step 3: Wallet Configuration  

In this step, you need to set up the WiFi, SNTP, IOTA node, and SEED.  

```
idf.py menuconfig

# WiFi SSID & Password
[IOTA Wallet] -> [WiFi]

# SNTP Client
[IOTA Wallet] -> [SNTP]

# Default IOTA node
[IOTA Wallet] -> [IOTA Node]

# Default IOTA SEED 
[IOTA Wallet] -> [IOTA Node] -> () Seed
```

You can check configures in `sdkconfig` file.  

Please make sure you assigned the seed(`CONFIG_IOTA_SEED`), Here is an example for your wallet configuration:  

```shell
CONFIG_SNTP_SERVER="pool.ntp.org"
CONFIG_SNTP_TZ="CST-8" 
CONFIG_IOTA_SEED="YOURSEED9YOURSEED9YOURSEED9YOURSEED9YOURSEED9YOURSEED9YOURSEED9YOURSEED9YOURSEED9"
CONFIG_IOTA_NODE_DEPTH=3
CONFIG_IOTA_NODE_MWM=14
CONFIG_IRI_NODE_URL="nodes.iota.cafe"
CONFIG_IRI_NODE_PORT=443
CONFIG_IOTA_NODE_ENABLE_HTTPS=y
CONFIG_WIFI_SSID="MY_SSID"
CONFIG_WIFI_PASSWORD="MY_PWD"
```

The `CONFIG_SNTP_TZ` follows the [POSIX Timezone string](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json)  

### Step 4: Build & flash

```shell
idf.py build
idf.py -p /dev/ttyUSB0 flash && idf.py -p /dev/ttyUSB0 monitor
```

Output:  
```shell
I (5149) tcpip_adapter: sta ip: 192.168.11.149, mask: 255.255.255.0, gw: 192.168.11.1
I (5149) esp32_main: Connected to AP
I (5149) esp32_main: IOTA Node: nodes.thetangle.org, port: 443, HTTPS:True

I (5159) esp32_main: Initializing SNTP: pool.ntp.org, Timezone: CST-8
I (5169) esp32_main: Waiting for system time to be set... (1/10)
I (7179) esp32_main: Waiting for system time to be set... (2/10)
I (9179) esp32_main: The current date/time is: Mon Jun 15 17:25:08 2020
IOTA> 
IOTA> node_info
=== Node: nodes.thetangle.org:443 ===
appName IRI 
appVersion 1.8.6 
latestMilestone: HQKJKH9QFEILIVFRVKINXDURQHPRZGWEXCQXIDZURDCAHWHYIIVSGSXMVYAAVNAGQJDDIDSLMMPIA9999
latestMilestoneIndex 1445037 
latestSolidSubtangleMilestone: HQKJKH9QFEILIVFRVKINXDURQHPRZGWEXCQXIDZURDCAHWHYIIVSGSXMVYAAVNAGQJDDIDSLMMPIA9999
latestSolidSubtangleMilestoneIndex 1445037 
neighbors 24 
packetsQueueSize 0 
time 1592213225856 
tips 6153 
transactionsToRequest 0 
IOTA> 
```

`help` for more details.  
`Ctrl` + `]` to exit.  

## Troubleshooting

`CONFIG_IOTA_SEED` is not set or is invalid:  
```shell
I (329) esp32_main: iota wallet system starting...
E (329) esp32_main: please set a valid seed in sdkconfig!
I (329) esp32_main: Restarting in 30 seconds...
I (1329) esp32_main: Restarting in 29 seconds...
```

`CONFIG_MAIN_TASK_STACK_SIZE` is too small, you need to enlarge it:  
```shell
***ERROR*** A stack overflow in task main has been detected.
abort() was called at PC 0x4008af7c on core 0
```
