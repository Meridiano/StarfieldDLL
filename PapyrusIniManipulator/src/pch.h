#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;
using namespace std::literals;

#include "../lib/mini/ini.h"
#define GetValue(RET,PTR) *std::bit_cast<RET*>(PTR)
#define GetValueEx(RET,PTR,INC) GetValue(RET,std::exchange(PTR,PTR+INC))
static const std::string space(1, ' ');
static const std::string tab(4, ' ');
