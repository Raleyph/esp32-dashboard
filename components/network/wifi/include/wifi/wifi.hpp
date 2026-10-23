// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include "esp_err.h"

namespace wifi {

    using ConnectedCallback = void (*)(void*);

    esp_err_t initialize(ConnectedCallback on_connected, void* arg);

}
