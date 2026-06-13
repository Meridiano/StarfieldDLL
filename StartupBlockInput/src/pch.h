#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

using namespace std::literals;

// ini reader
#include "../lib/mini/ini.h"

// required
#include "windows.h"
#define ThreadID std::to_string(std::bit_cast<std::uint32_t>(std::this_thread::get_id()))
#undef min
#undef max
