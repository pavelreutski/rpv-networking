#pragma once

#include <stdint.h>

bool httpserver_start(uint16_t port);
bool httpserver_shutdown(void);