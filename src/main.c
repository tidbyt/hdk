#include <assets.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <webp/demux.h>

#include "audio.h"
#include "display.h"
#include "flash.h"
#include "gfx.h"
#include "remote.h"
#include "sdkconfig.h"
#include "touch.h"
#include "wifi.h"

static const char* TAG = "main";

void _on_touch() {
  ESP_LOGI(TAG, "Touch detected");
  audio_play(ASSET_LAZY_DADDY_MP3, ASSET_LAZY_DADDY_MP3_LEN);
}

void app_main(void) {
  ESP_LOGI(TAG, "Hello world!");

  // Setup the device flash storage.
  if (flash_initialize()) {
    ESP_LOGE(TAG, "failed to initialize flash");
    return;
  }
  esp_register_shutdown_handler(&flash_shutdown);

  // Setup the display.
  if (gfx_initialize(ASSET_NOAPPS_WEBP, ASSET_NOAPPS_WEBP_LEN)) {
    ESP_LOGE(TAG, "failed to initialize gfx");
    return;
  }
  esp_register_shutdown_handler(&display_shutdown);

  // Setup WiFi.
  if (wifi_initialize(TIDBYT_WIFI_SSID, TIDBYT_WIFI_PASSWORD)) {
    ESP_LOGE(TAG, "failed to initialize WiFi");
    return;
  }
  esp_register_shutdown_handler(&wifi_shutdown);

  // Setup audio.
  if (audio_initialize() != ESP_OK) {
    ESP_LOGE(TAG, "failed to initialize audio");
    return;
  }

  // Setup touch.
  if (touch_initialize(_on_touch) != ESP_OK) {
    ESP_LOGE(TAG, "failed to initialize touch");
    return;
  }

  uint8_t mac[6];
  if (!wifi_get_mac(mac)) {
    ESP_LOGI(TAG, "WiFi MAC: %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1],
             mac[2], mac[3], mac[4], mac[5]);
  }

  // Play a sample. This will only have an effect on Gen 2 devices.
  audio_play(ASSET_LAZY_DADDY_MP3, ASSET_LAZY_DADDY_MP3_LEN);

  for (;;) {
    uint8_t* webp;
    size_t len;
    if (remote_get(TIDBYT_REMOTE_URL, &webp, &len)) {
      ESP_LOGE(TAG, "Failed to get webp");
    } else {
      ESP_LOGI(TAG, "Updated webp (%d bytes)", len);
      gfx_update(webp, len);
      free(webp);
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
