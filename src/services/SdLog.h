#pragma once

#include <Arduino.h>

namespace sdlog {

void begin();
void log(const char* tag, const String& msg);
void section(const char* tag);

} // namespace sdlog
