#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;
using namespace std::literals;

#include "../lib/mini/ini.h"
#define GetU8(PTR) *std::bit_cast<std::uint8_t*>(PTR)
#define GetU16(PTR) *std::bit_cast<std::uint16_t*>(PTR)
static const std::string space(1, ' ');
static const std::string tab(4, ' ');
