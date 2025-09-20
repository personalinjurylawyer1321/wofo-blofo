#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/uart.h>
#include <cJSON.h>
#include <driver/i2c.h>
#include <ssd1306.h>
#include <driver/adc.h>
#include "esp_bt.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define TAG "Simulator"
#define UART_NUM UART_NUM_1
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSID_COUNT 10
#define MAX_NETWORKS 50
#define BUFFER_SIZE 1024 // Reduced from 8192 to fit stack better

typedef struct {
    char ssid[32];
    uint8_t mac[6];
    int channel;
    int rssi;
    char type[8];
} Network;

static Network *networks = NULL;
static int num_networks = 0, bt_count = 0;
static bool live = false;
static struct ble_hs_adv_fields adv_fields;
static SemaphoreHandle_t ble_semaphore = NULL;

static void display_status(const char *msg);
static void rotate_ble(void);

static uint8_t beacon_frame_template[] = {
    0x80, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x64, 0x00,
    0x01, 0x04,
    0x00, 0x00,
    0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,
    0x03, 0x01, 0x01
};

static void send_beacon_frame(Network *net) {
    if (!net) return;

    uint8_t frame[128];
    memcpy(frame, beacon_frame_template, sizeof(beacon_frame_template));
    memcpy(&frame[10], net->mac, 6);
    memcpy(&frame[16], net->mac, 6);
    size_t ssid_len = strlen(net->ssid);
    if (ssid_len > 32) ssid_len = 32; // Cap SSID length
    frame[38] = ssid_len;
    memcpy(&frame[39], net->ssid, ssid_len);
    frame[50] = net->channel;
    int tx_power = (net->rssi + 90) * (78 - 8) / (50 - 90) + 8; // Maps RSSI (-90 to -50) to tx_power (8 to 78)
    tx_power = (tx_power < 8) ? 8 : (tx_power > 78) ? 78 : tx_power;
    esp_wifi_set_max_tx_power(tx_power);
    ESP_LOGI(TAG, "Beacon: %s, Ch %d, RSSI %d, TX Power %d", net->ssid, net->channel, net->rssi, tx_power);

    // Note: esp_wifi_80211_tx is deprecated in v5.5. Consider using AP mode beacon configuration instead.
    // For now, proceed with caution:
    esp_wifi_80211_tx(WIFI_IF_AP, frame, sizeof(beacon_frame_template) + ssid_len, false);
}

static void ble_advertise(void *arg) {
    while (1) {
        if (xSemaphoreTake(ble_semaphore, portMAX_DELAY) == pdTRUE) {
            if (live && bt_count > 0) {
                rotate_ble();
            }
            xSemaphoreGive(ble_semaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(random() % 50 + 50)); // Random delay 50-100ms
    }
}

static void rotate_ble(void) {
    int idx = num_networks + (bt_count ? (random() % bt_count) : 0);
    Network *net = &networks[idx];
    if (strcmp(net->type, "BLE") != 0) return;

    memset(&adv_fields, 0, sizeof(adv_fields));
    adv_fields.name = (uint8_t *)net->ssid;
    adv_fields.name_len = strlen(net->ssid);
    adv_fields.name_is_complete = 1;

    char mdata[16];
    snprintf(mdata, sizeof(mdata), "SIM%d", random() % 9000 + 1000);
    adv_fields.mfg_data = (uint8_t *)mdata;
    adv_fields.mfg_data_len = strlen(mdata);

    ble_uuid_t uuid = BLE_UUID16_DECLARE(0x180F);
    uint8_t svc_data[] = {0x01};
    size_t svc_data_len = sizeof(uuid.u16.value) + sizeof(svc_data);
    uint8_t *svc_data_buf = malloc(svc_data_len);
    if (svc_data_buf) {
        memcpy(svc_data_buf, &uuid.u16.value, sizeof(uuid.u16.value));
        memcpy(svc_data_buf + sizeof(uuid.u16.value), svc_data, sizeof(svc_data));
        adv_fields.svc_data_uuid16 = svc_data_buf;
        adv_fields.svc_data_uuid16_len = svc_data_len;
    }

    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_NON,
        .disc_mode = BLE_GAP_DISC_MODE_NON,
        .itvl_min = 100, // 100ms
        .itvl_max = 200  // 200ms
    };
    int rc = ble_gap_adv_start(0, &adv_params, BLE_HS_FOREVER, &adv_fields, BLE_HS_ADV_DEF_FILT, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "BLE advertising start failed: %d", rc);
    } else {
        ESP_LOGI(TAG, "BLE: %s", net->ssid);
    }

    vTaskDelay(pdMS_TO_TICKS(random() % 50 + 50));
    ble_gap_adv_stop();
    free(svc_data_buf); // Free allocated memory
}

static void ble_host_task(void *param) {
    nimble_port_run(); // Main NimBLE event loop
}

void app_main(void) {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Bluetooth controller
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    // Initialize NimBLE
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NimBLE port init failed: %d", ret);
        return;
    }

    // Initialize NimBLE host
    ret = ble_hs_init();
    if (ret != 0) {
        ESP_LOGE(TAG, "BLE host init failed: %d", ret);
        return;
    }

    ble_hs_cfg.sync_cb = ble_hs_sync_cb;
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_hs_cfg.sm_io_cap = BLE_HS_IO_NO_INPUT_OUTPUT;
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set("Simulator"));

    // Create semaphore for BLE synchronization
    ble_semaphore = xSemaphoreCreateMutex();
    if (ble_semaphore == NULL) {
        ESP_LOGE(TAG, "Failed to create BLE semaphore");
        return;
    }

    // Allocate dynamic memory
    networks = malloc(MAX_NETWORKS * sizeof(Network));
    char *buffer = malloc(BUFFER_SIZE);
    if (!networks || !buffer) {
        ESP_LOGE(TAG, "Memory allocation failed");
        free(networks);
        free(buffer);
        return;
    }

    // Initialize UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, 16, 17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUFFER_SIZE, 0, 0, NULL, 0));

    // Initialize I2C for OLED
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, i2c_config.mode, 0, 0, 0));
    ESP_ERROR_CHECK(ssd1306_init(I2C_MASTER_NUM, SCREEN_WIDTH, SCREEN_HEIGHT, 0x3C));
    ssd1306_clear();
    display_status("Simulator Ready");

    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(8)); // Initial power
    ESP_ERROR_CHECK(esp_wifi_start());

    // Create tasks
    xTaskCreate(ble_host_task, "ble_host_task", 4096, NULL, 5, NULL);
    xTaskCreate(ble_advertise, "ble_advertise", 4096, NULL, 5, NULL);

    // Initialize ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // GPIO 34, if used

    // Main loop for UART processing
    int buffer_pos = 0;
    while (1) {
        uint8_t data;
        int len = uart_read_bytes(UART_NUM, &data, 1, pdMS_TO_TICKS(100));
        if (len > 0) {
            if (data == '\n' || buffer_pos >= BUFFER_SIZE - 1) {
                buffer[buffer_pos] = '\0';
                cJSON *root = cJSON_Parse(buffer);
                if (root) {
                    cJSON *results = cJSON_GetObjectItem(root, "results");
                    if (results) {
                        num_networks = 0;
                        bt_count = 0;
                        int total_items = cJSON_GetArraySize(results);
                        for (int i = 0; i < total_items && num_networks + bt_count < MAX_NETWORKS; i++) {
                            cJSON *net = cJSON_GetArrayItem(results, i);
                            cJSON *type_item = cJSON_GetObjectItem(net, "type");
                            const char *type = type_item ? type_item->valuestring : "WIFI";
                            if (strcmp(type, "WIFI") == 0 && num_networks < SSID_COUNT) {
                                cJSON *ssid_item = cJSON_GetObjectItem(net, "ssid");
                                strncpy(networks[num_networks].ssid, ssid_item ? ssid_item->valuestring : "N/A", 32);
                                cJSON *mac_item = cJSON_GetObjectItem(net, "netid");
                                const char *mac_str = mac_item ? mac_item->valuestring : "00:11:22:33:44:55";
                                sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                                       &networks[num_networks].mac[0], &networks[num_networks].mac[1],
                                       &networks[num_networks].mac[2], &networks[num_networks].mac[3],
                                       &networks[num_networks].mac[4], &networks[num_networks].mac[5]);
                                cJSON *channel_item = cJSON_GetObjectItem(net, "channel");
                                networks[num_networks].channel = channel_item ? channel_item->valueint : (rand() % 13 + 1);
                                cJSON *rssi_item = cJSON_GetObjectItem(net, "rssi");
                                networks[num_networks].rssi = rssi_item ? rssi_item->valueint : (rand() % 41 - 90);
                                strcpy(networks[num_networks].type, "WIFI");
                                num_networks++;
                            } else if (strcmp(type, "BLE") == 0 && bt_count < MAX_NETWORKS - num_networks) {
                                cJSON *ssid_item = cJSON_GetObjectItem(net, "ssid");
                                snprintf(networks[num_networks + bt_count].ssid, 32, "%s",
                                         ssid_item ? ssid_item->valuestring : "BLE_Device_%d", rand() % 900 + 100);
                                cJSON *mac_item = cJSON_GetObjectItem(net, "netid");
                                const char *mac_str = mac_item ? mac_item->valuestring : "00:11:22:33:44:55";
                                sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                                       &networks[num_networks + bt_count].mac[0], &networks[num_networks + bt_count].mac[1],
                                       &networks[num_networks + bt_count].mac[2], &networks[num_networks + bt_count].mac[3],
                                       &networks[num_networks + bt_count].mac[4], &networks[num_networks + bt_count].mac[5]);
                                cJSON *rssi_item = cJSON_GetObjectItem(net, "rssi");
                                networks[num_networks + bt_count].rssi = rssi_item ? rssi_item->valueint : (rand() % 41 - 90);
                                strcpy(networks[num_networks + bt_count].type, "BLE");
                                bt_count++;
                            }
                        }
                    }
                    cJSON_Delete(root);
                    live = true;
                    display_status("Starting...");
                }
                buffer_pos = 0;
            } else {
                buffer[buffer_pos++] = data;
            }
        }
        if (live && num_networks > 0) {
            if (xSemaphoreTake(ble_semaphore, pdMS_TO_TICKS(10)) == pdTRUE) {
                for (int i = 0; i < SSID_COUNT && i < num_networks; i++) {
                    send_beacon_frame(&networks[i]);
                    vTaskDelay(pdMS_TO_TICKS(10)); // Short delay between beacons
                }
                xSemaphoreGive(ble_semaphore);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Main loop delay
    }

    // Cleanup on exit (unreachable in this loop, but good practice)
    free(networks);
    free(buffer);
    vSemaphoreDelete(ble_semaphore);
}

static void display_status(const char *msg) {
    if (!msg) return;
    ssd1306_clear();
    ssd1306_draw_string(0, 0, (char *)msg, 1);
    if (live) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "WiFi: %d", num_networks);
        ssd1306_draw_string(0, 16, buffer, 1);
        snprintf(buffer, sizeof(buffer), "BLE: %d", bt_count);
        ssd1306_draw_string(0, 24, buffer, 1);
        if (num_networks > 0) {
            ssd1306_draw_string(0, 32, networks[0].ssid, 1); // Display first SSID
        }
    }
    ssd1306_display();
}
