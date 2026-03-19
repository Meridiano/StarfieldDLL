#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

using namespace std::literals;

// custom
#include "../lib/mini/ini.h"
#define GetU8(PTR) *std::bit_cast<std::uint8_t*>(PTR)
#define GetU16(PTR) *std::bit_cast<std::uint16_t*>(PTR)
