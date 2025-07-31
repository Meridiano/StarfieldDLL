#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;

using namespace std::literals;
using StrVec = std::vector<std::string>;

#include <windows.h>
#include "toml++/toml.h"
#include "glfw/glfw3.h"
#define TRAMPOLINE REL::GetTrampoline()
