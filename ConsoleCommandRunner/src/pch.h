#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;

using namespace std::literals;
using StrVec = std::vector<std::string>;

#include "toml++/toml.h"
#define TRAMPOLINE REL::GetTrampoline()
