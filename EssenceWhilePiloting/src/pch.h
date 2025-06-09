#pragma once

#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

#define GetByte(PTR) *std::bit_cast<std::uint8_t*>(PTR)
