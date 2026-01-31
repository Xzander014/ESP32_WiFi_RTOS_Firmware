#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_system.h"

/* ------------------ CONFIG ------------------ */
#define WIFI_SSID "Kay"
#define WIFI_PASS "aaaaasssss"

#define QUEUE_DEPTH 5
#define NVS_NAMESPACE "cfg"

/* ------------------ TAG ------------------ */
static const char *TAG = "APP";

/* ------------------ RTOS HANDLES ------------------ */
static QueueHandle_t cmdQueue;
static SemaphoreHandle_t logMutex;

/* ------------------ STATE MACHINE ------------------ */
typedef enum {
  SYS_INIT,
  SYS_WIFI_CONNECTING,
  SYS_WIFI_CONNECTED,
  SYS_SERVER_RUNNING,
  SYS_ERROR
} system_state_t;

static system_state_t systemState;

/* ------------------ COMMAND FRAMEWORK ------------------ */
typedef enum {
  CMD_NOP = 0,
  CMD_LED_ON,
  CMD_LED_OFF,
  CMD_GET_STATUS,
  CMD_REBOOT
} command_id_t;

typedef struct {
  command_id_t cmd;
  uint32_t value;
} command_msg_t;

/* ------------------ PERSISTENT CONFIG ------------------ */
typedef struct {
  uint32_t boot_count;
} persistent_cfg_t;

static persistent_cfg_t g_cfg;

/* ------------------ NVS ------------------ */
static void load_config(void) {
  nvs_handle_t handle;

  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
    size_t size = sizeof(g_cfg);

    if (nvs_get_blob(handle, "cfg", &g_cfg, &size) != ESP_OK) {
      memset(&g_cfg, 0, sizeof(g_cfg));
    }

    g_cfg.boot_count++;

    nvs_set_blob(handle, "cfg", &g_cfg, sizeof(g_cfg));
    nvs_commit(handle);
    nvs_close(handle);
  }
  m
}

/* ------------------ WIFI ------------------ */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    systemState = SYS_WIFI_CONNECTING;
    esp_wifi_connect();
  }

  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    systemState = SYS_WIFI_CONNECTED;
  }
}

static void wifi_init_sta(void) {
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler,
                             NULL);

  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler,
                             NULL);

  wifi_config_t wifi_config = {
      .sta = {.ssid = WIFI_SSID, .password = WIFI_PASS}};

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();
  esp_wifi_connect();

  systemState = SYS_WIFI_CONNECTING;
}

/* ------------------ HTTP ------------------ */
static esp_err_t command_get_handler(httpd_req_t *req) {
  char query[64];
  char cmd_str[16];

  if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing query");
    return ESP_FAIL;
  }

  if (httpd_query_key_value(query, "cmd", cmd_str, sizeof(cmd_str)) != ESP_OK) {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing cmd");
    return ESP_FAIL;
  }

  command_msg_t msg = {0};
  msg.cmd = atoi(cmd_str);

  if (uxQueueSpacesAvailable(cmdQueue) == 0) {
    httpd_resp_send_err(req, HTTPD_503_SERVICE_UNAVAILABLE, "Queue Full");
    return ESP_FAIL;
  }

  xQueueSend(cmdQueue, &msg, 0);
  httpd_resp_sendstr(req, "Command accepted");
  return ESP_OK;
}

static void start_http_server(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_uri_t uri = {
        .uri = "/cmd", .method = HTTP_GET, .handler = command_get_handler};

    httpd_register_uri_handler(server, &uri);
    systemState = SYS_SERVER_RUNNING;
  }
}

/* ------------------ TASKS ------------------ */
static void processor_task(void *arg) {
  command_msg_t msg;

  while (1) {
    if (xQueueReceive(cmdQueue, &msg, portMAX_DELAY)) {
      xSemaphoreTake(logMutex, portMAX_DELAY);

      switch (msg.cmd) {
      case CMD_LED_ON:
        ESP_LOGI("PROC", "LED ON");
        break;

      case CMD_LED_OFF:
        ESP_LOGI("PROC", "LED OFF");
        break;

      case CMD_GET_STATUS:
        ESP_LOGI("PROC", "Boot count: %lu", g_cfg.boot_count);
        break;

      case CMD_REBOOT:
        ESP_LOGI("PROC", "Rebooting...");
        esp_restart();
        break;

      default:
        ESP_LOGW("PROC", "Unknown command");
        break;
      }

      xSemaphoreGive(logMutex);
    }
  }
}

static void logger_task(void *arg) {
  while (1) {
    xSemaphoreTake(logMutex, portMAX_DELAY);

    ESP_LOGI("LOG", "State=%d Heap=%lu", systemState, esp_get_free_heap_size());

    xSemaphoreGive(logMutex);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

/* ------------------ MAIN ------------------ */
void app_main(void) {
  nvs_flash_init();
  load_config();

  cmdQueue = xQueueCreate(QUEUE_DEPTH, sizeof(command_msg_t));
  logMutex = xSemaphoreCreateMutex();

  systemState = SYS_INIT;

  wifi_init_sta();

  while (systemState != SYS_WIFI_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(200));
  }

  start_http_server();

  xTaskCreatePinnedToCore(processor_task, "processor", 4096, NULL, 2, NULL, 1);

  xTaskCreatePinnedToCore(logger_task, "logger", 4096, NULL, 1, NULL, 0);
}
