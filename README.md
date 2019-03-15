# IOTA CClient example on ESP32

This is an example project to demonstrate how to use [CClient](https://github.com/iotaledger/entangled/tree/develop/cclient) on the ESP32 development framework.

## Requirement  

* ESP32 toolchain
* ESP-IDF

To get the toolchain and ESP-IDF, please reference [esp-idf with CMake](https://docs.espressif.com/projects/esp-idf/en/latest/get-started-cmake/index.html#installation-step-by-step).

## Flash to ESP32

### Step 1: checkout source  

```
git clone --recursive https://github.com/oopsmonk/iota_cclient_esp32.git
```
or
```
git clone https://github.com/oopsmonk/iota_cclient_esp32.git
cd iota_cclient_esp32
git submodule update --init
```

### Step 2: Init components

```
cd iota_cclient_esp32
bash ./init.sh
```

### Step 3: Configure 

```
idf.py menuconfig
# configure WiFi SSID & Password
[IOTA CClient Configuration] -> [WiFi SSID]
[IOTA CClient Configuration] -> [WiFi Password]
```

### Step 4: Build & flash

```
idf.py build
idf.py -p ${ESP32_PORT} flash && idf.py -p ${ESP32_PORT} monitor
```

output:  
```
I (1522) wifi: pm start, type: 1

I (2212) event: sta ip: 192.168.11.7, mask: 255.255.255.0, gw: 192.168.11.1
I (2212) iota_main: Connected to AP
I (2212) iota_main: IRI Node: nodes.thetangle.org, port: 443, HTTPS:True

appName IRI
appVersion 1.6.1-RELEASE
latestMilestone SHOGJGUFOMKVJJVHMVPBHSUFFWAHUEGAKBHYTJXYNRAHAPDFLMJWOJBQSPYUNPYKQUUXPSYD9SUHZ9999
latestMilestoneIndex 1026336
latestSolidSubtangleMilestone SHOGJGUFOMKVJJVHMVPBHSUFFWAHUEGAKBHYTJXYNRAHAPDFLMJWOJBQSPYUNPYKQUUXPSYD9SUHZ9999
latestSolidSubtangleMilestoneIndex 1026336
milestoneStratIndex 1022912
neighbors 24
packetsQueueSize 0
time 1552658125320
tips 5042
transactionsToRequest 0
I (4912) iota_main: Restarting in 30 seconds...
I (5922) iota_main: Restarting in 29 seconds...
I (6922) iota_main: Restarting in 28 seconds...
```
