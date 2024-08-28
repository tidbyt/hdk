#pragma once

esp_err_t audio_initialize();
esp_err_t audio_play(const unsigned char *data, size_t length);
