#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace logs = SFSE::log;
namespace fs = std::filesystem;
using namespace std::literals;

#include "xbyak/xbyak.h"
#include "../lib/mini/ini.h"

#define TRAMPOLINE REL::GetTrampoline()
