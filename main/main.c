// IOTA CClient Example

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "cclient/types/types.h"
#include <inttypes.h>

static char const *amazon_ca1_pem =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
    "-----END CERTIFICATE-----\r\n";

static const char *TAG = "iota_main";

static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const static int CONNECTED_BIT = BIT0;

static void init_iota_client(iota_client_service_t *const service)
{
  service->http.path = "/";
  service->http.content_type = "application/json";
  service->http.accept = "application/json";
  service->http.host = CONFIG_IRI_NODE_URI;
  service->http.port = atoi(CONFIG_IRI_NODE_PORT);
  service->http.api_version = 1;
#ifdef CONFIG_ENABLE_HTTPS
  service->http.ca_pem = amazon_ca1_pem;
#else
  service->http.ca_pem = NULL;
#endif
  service->serializer_type = SR_JSON;
  logger_init();
  logger_output_register(stdout);
  logger_output_level_set(stdout, LOGGER_DEBUG);
  iota_client_core_init(service);
  iota_client_extended_init();
}

static retcode_t test_get_node_info(iota_client_service_t *const service)
{
  retcode_t ret = RC_ERROR;
  // test get_node_info
  trit_t trytes_out[NUM_TRYTES_HASH + 1];
  size_t trits_count = 0;
  get_node_info_res_t *node_res = get_node_info_res_new();
  if (!node_res)
  {
    return RC_OOM;
  }

  ret = iota_client_get_node_info(service, node_res);
  if (ret == RC_OK)
  {
    printf("appName %s \n", node_res->app_name->data);
    printf("appVersion %s \n", node_res->app_version->data);
    trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH,
                                       node_res->latest_milestone, NUM_TRITS_HASH,
                                       NUM_TRITS_HASH);
    if (trits_count == 0)
    {
      printf("trit converting failed\n");
      goto done;
    }
    trytes_out[NUM_TRYTES_HASH] = '\0';
    printf("latestMilestone %s \n", trytes_out);
    printf("latestMilestoneIndex %zu\n", (uint32_t)node_res->latest_milestone_index);
    trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_HASH,
                                       node_res->latest_milestone, NUM_TRITS_HASH,
                                       NUM_TRITS_HASH);
    if (trits_count == 0)
    {
      printf("trit converting failed\n");
      goto done;
    }
    trytes_out[NUM_TRYTES_HASH] = '\0';
    printf("latestSolidSubtangleMilestone %s \n", trytes_out);

    printf("latestSolidSubtangleMilestoneIndex %zu\n",
           node_res->latest_solid_subtangle_milestone_index);
    printf("milestoneStratIndex %zu\n", node_res->milestone_start_index);
    printf("neighbors %d \n", node_res->neighbors);
    printf("packetsQueueSize %d \n", node_res->packets_queue_size);
    printf("time %" PRIu64 "\n", node_res->time);
    printf("tips %zu \n", node_res->tips);
    printf("transactionsToRequest %zu\n", node_res->transactions_to_request);
  }
  else
  {
    ESP_LOGE(TAG, "get_node_info error:%s", error_2_string(ret));
  }

done:
  get_node_info_res_free(&node_res);
  return ret;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
    esp_wifi_connect();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
    esp_wifi_connect();
    xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    break;
  default:
    break;
  }
  return ESP_OK;
}

static void wifi_conn_init(void)
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {
      .sta = {
          .ssid = CONFIG_WIFI_SSID,
          .password = CONFIG_WIFI_PASSWORD,
      },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
}

static void iota_client_task(void *p)
{

  while (1)
  {
    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                        false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
    ESP_LOGI(TAG, "IRI Node: %s, port: %s, HTTPS:%s\n", CONFIG_IRI_NODE_URI,
            CONFIG_IRI_NODE_PORT, CONFIG_ENABLE_HTTPS ? "True" : "False");
    iota_client_service_t client_service;
    // init cclient stuff
    init_iota_client(&client_service);
    // calling iota_client_get_node_info
    test_get_node_info(&client_service);

    for (int i = 30; i >= 0; i--)
    {
      ESP_LOGI(TAG, "Restarting in %d seconds...", i);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Restarting now.\n");
    fflush(stdout);
    esp_restart();
  }
}
void app_main()
{
  ESP_LOGI(TAG, "app_main starting...");

  /* Print chip information */
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

  ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  ESP_ERROR_CHECK(nvs_flash_init());
  wifi_conn_init();
  xTaskCreate(iota_client_task, "iota_client", 20480, NULL, 5, NULL);
}
