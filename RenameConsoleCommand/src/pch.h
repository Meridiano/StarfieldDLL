#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

using namespace std::literals;

// custom
#include "../lib/mini/ini.h"
#define GetValue(RET,PTR) *std::bit_cast<RET*>(PTR)
#define GetValueEx(RET,PTR,INC) GetValue(RET,std::exchange(PTR,PTR+INC))
