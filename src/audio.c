#ifdef TIDBYT_GEN2

#include <audio_player.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/task.h>

static const char *TAG = "audio";

#define I2S_NUM (I2S_NUM_0)
#define I2S_SAMPLE_RATE (44100)

#define I2S_PINS                                                    \
  {                                                                 \
    .mclk = (gpio_num_t)-1, .bclk = GPIO_NUM_12, .ws = GPIO_NUM_13, \
    .dout = GPIO_NUM_14, .din = (gpio_num_t)-1,                     \
    .invert_flags = {                                               \
        .mclk_inv = false,                                          \
        .bclk_inv = false,                                          \
        .ws_inv = false,                                            \
    },                                                              \
  }

static i2s_chan_handle_t _tx;

static esp_err_t _setI2SClock(uint32_t rate, uint32_t bitWidth,
                              i2s_slot_mode_t mode) {
  ESP_LOGI(TAG, "setI2SClock: rate=%lu, bitWidth=%lu, mode=%d", rate, bitWidth,
           mode);

  esp_err_t ret = ESP_OK;

  i2s_std_config_t config = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(rate),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
          (i2s_data_bit_width_t)bitWidth, mode),
      .gpio_cfg = I2S_PINS,
  };

  ret |= i2s_channel_disable(_tx);
  ret |= i2s_channel_reconfig_std_clock(_tx, &config.clk_cfg);
  ret |= i2s_channel_reconfig_std_slot(_tx, &config.slot_cfg);
  ret |= i2s_channel_enable(_tx);

  return ret;
}

static esp_err_t _i2sWrite(void *buf, size_t len, size_t *bytesWritten,
                           uint32_t timeout) {
  if (len % 2 != 0) {
    return ESP_ERR_INVALID_SIZE;
  }

  return i2s_channel_write(_tx, (uint8_t *)buf, len, bytesWritten, timeout);
}

static esp_err_t _initI2S() {
  esp_err_t ret = ESP_OK;

  i2s_std_config_t config = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
      .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,
                                                  I2S_SLOT_MODE_STEREO),
      .gpio_cfg = I2S_PINS};

  i2s_chan_config_t channels =
      I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
  channels.dma_frame_num *= 2;
  channels.auto_clear = true;
  ret = i2s_new_channel(&channels, &_tx, NULL);
  if (ret != ESP_OK) {
    return ret;
  }

  ret |= i2s_channel_init_std_mode(_tx, &config);
  ret |= i2s_channel_enable(_tx);

  return ret;
}

static esp_err_t _mute_cb(AUDIO_PLAYER_MUTE_SETTING setting) {
  ESP_LOGI(TAG, "Mute setting: %d", setting);
  return ESP_OK;
}

static void _shutdown() {
  audio_player_delete();
  i2s_channel_disable(_tx);
  i2s_del_channel(_tx);
}

esp_err_t audio_initialize() {
  esp_err_t ret = _initI2S();
  if (ret != ESP_OK) {
    return ret;
  }

  audio_player_config_t config = {
      .mute_fn = _mute_cb,
      .clk_set_fn = _setI2SClock,
      .write_fn = _i2sWrite,
      .priority = configMAX_PRIORITIES - 1,
      .coreID = tskNO_AFFINITY,
  };
  ret = audio_player_new(config);
  if (ret != ESP_OK) {
    return ret;
  }

  esp_register_shutdown_handler(&_shutdown);

  return ESP_OK;
}

esp_err_t audio_play(const unsigned char *data, size_t length) {
  FILE *fp = fmemopen((void *)data, length, "rb");
  assert(fp != NULL);

  return audio_player_play(fp);
}

#else

#include <esp_err.h>
#include <stddef.h>

esp_err_t audio_play(const unsigned char *data, size_t length) {
  return ESP_OK;
}

esp_err_t audio_initialize() { return ESP_OK; }

#endif
