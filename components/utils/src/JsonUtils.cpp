// Copyright (c) 2026 Raleyph. All rights reserved.

#include "utils/JsonUtils.hpp"

namespace JsonUtils {

void add_string(cJSON* object, const char* key, const char* value) {
    if (value) {
        cJSON_AddStringToObject(object, key, value);
    } else {
        cJSON_AddNullToObject(object, key);
    }
}

void add_boolean(cJSON* object, const char* key, const bool value) {
    cJSON_AddBoolToObject(object, key, value);
}

}
