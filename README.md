# IOTA CClient example on ESP32

## Requirement  

[esp-idf with CMake](https://docs.espressif.com/projects/esp-idf/en/latest/get-started-cmake/index.html)  

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
I (4663) event: sta ip: 192.168.11.149, mask: 255.255.255.0, gw: 192.168.11.1
I (4663) iota_main: Connected to AP
appName IRI Testnet 
appVersion 1.5.6-RELEASE 
latestMilestone 9HAEKWTNQKDCZOCKRCUMSMYWLKNORAITPUVFYIRULIYJOGRBRY9DKSJOITKYFUFWPBVXZKRGHEPTLR999 
latestMilestoneIndex 1144104
latestSolidSubtangleMilestone 9HAEKWTNQKDCZOCKRCUMSMYWLKNORAITPUVFYIRULIYJOGRBRY9DKSJOITKYFUFWPBVXZKRGHEPTLR999 
latestSolidSubtangleMilestoneIndex 1144104
milestoneStratIndex 434525
neighbors 3 
packetsQueueSize 0 
time 1551149072563
tips 301 
transactionsToRequest 0
I (5613) iota_main: Restarting in 30 seconds...
I (6613) iota_main: Restarting in 29 seconds...
I (7613) iota_main: Restarting in 28 seconds...
```
