#pragma once

#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

// custom
#include "glaze/json.hpp"
#define SleepFor(N,T) std::this_thread::sleep_for(std::chrono::T(N))
#define ScopedEnum(N,T,...) struct N##Scope { enum N##Enum : T { __VA_ARGS__ }; }; using N = N##Scope::N##Enum
