#pragma once

#include <esp_err.h>

typedef void (*touch_callback_t)();

esp_err_t touch_initialize(touch_callback_t callback);
