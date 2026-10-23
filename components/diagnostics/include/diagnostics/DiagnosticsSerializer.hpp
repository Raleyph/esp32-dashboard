// Copyright (c) 2026 Raleyph. All rights reserved.

#pragma once

#include "cJSON.h"

#include "diagnostics/DiagnosticsMetrics.hpp"

namespace logging {

struct LogMessage;

}

namespace diagnostics::serialization {

void add_partition(cJSON* parent, const char* key, const PartitionInfo& partition);

char* serialize_system_info(const SystemInfo& info);
char* serialize_dynamic_metrics(const SystemMetrics& metrics);
char* serialize_log(const logging::LogMessage& message);

}
