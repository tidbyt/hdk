#include "touch.h"

#include <driver/gpio.h>
#include <driver/touch_pad.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>

#define TOUCH_PAD_NUM TOUCH_PAD_NUM8
#define DEBOUNCE_TIME_MICROS (250 * 1000)
#define TOUCH_FILTER_PERIOD_MILLIS 10

const char *TAG = "touch";

touch_callback_t _callback = NULL;
uint16_t _threshold = 0;
uint64_t _lastTouch = 0;

static void _filterCallback(uint16_t *raw, uint16_t *filtered) {
  uint16_t val = filtered[TOUCH_PAD_NUM];

  uint64_t now = esp_timer_get_time();
  if (val < _threshold && now - _lastTouch > DEBOUNCE_TIME_MICROS) {
    _lastTouch = now;
    if (_callback != NULL) {
      _callback();
    }
  }
}

esp_err_t touch_initialize(touch_callback_t callback) {
  _callback = callback;

  esp_err_t err = touch_pad_init();
  if (err != ESP_OK) {
    return err;
  }

  touch_pad_set_measurement_interval(TOUCH_PAD_SLEEP_CYCLE_DEFAULT / 4);
  touch_pad_set_measurement_clock_cycles(TOUCH_PAD_MEASURE_CYCLE_DEFAULT / 2);

  err = touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
  if (err != ESP_OK) {
    return err;
  }

  err = touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5,
                              TOUCH_HVOLT_ATTEN_1V);
  if (err != ESP_OK) {
    return err;
  }

  err = touch_pad_filter_start(TOUCH_FILTER_PERIOD_MILLIS);
  if (err != ESP_OK) {
    return err;
  }

  err = touch_pad_config(TOUCH_PAD_NUM, 0);
  if (err != ESP_OK) {
    return err;
  }

  ESP_LOGI(TAG, "Reading current touch pad values");
  uint16_t thresh = 0;
  for (int i = 0; i < 3; i++) {
    vTaskDelay(pdMS_TO_TICKS(100));
    uint16_t x;
    touch_pad_read_filtered(TOUCH_PAD_NUM, &x);

    if (x > thresh) {
      thresh = x;
    }
  }

  _threshold = (thresh * 95) / 100;

  ESP_LOGI(TAG, "Touch threshold: %d", _threshold);

  err = touch_pad_config(TOUCH_PAD_NUM, 0);
  if (err != ESP_OK) {
    return err;
  }

  err = touch_pad_set_filter_read_cb(_filterCallback);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}
