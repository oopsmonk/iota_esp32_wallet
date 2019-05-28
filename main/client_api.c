#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "client_api.h"

static const char *TAG = "cclient";

void init_iota_client(iota_client_service_t *const service)
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

retcode_t test_get_node_info(iota_client_service_t *const service)
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

retcode_t test_iota_client(iota_client_service_t *const service)
{
	init_iota_client(service);
	return test_get_node_info(service);
}