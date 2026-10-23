// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include <cstdint>
#include <optional>
#include <type_traits>

#include "cJSON.h"

namespace JsonUtils {

template<typename T>
void add_number(cJSON* object, const char* key, T value) {
    static_assert(
        std::is_arithmetic_v<T> && !std::is_same_v<T, bool>,
        "T must be a numeric type"
    );

    cJSON_AddNumberToObject(object, key, static_cast<double>(value));
}

template<typename T>
void add_number(cJSON* object, const char* key, const std::optional<T>& value) {
    if (value.has_value()) {
        add_number(object, key, *value);
    } else {
        cJSON_AddNullToObject(object, key);
    }
}

void add_string(cJSON* object, const char* key, const char* value);
void add_boolean(cJSON* object, const char* key, bool value);

}
