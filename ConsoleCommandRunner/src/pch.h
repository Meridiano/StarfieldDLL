#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;

using namespace std::literals;
using StrVec = std::vector<std::string>;

#include <windows.h>
#include "toml++/toml.h"
#include "xbyak/xbyak.h"
#include "glfw/glfw3.h"

#define TRAMPOLINE REL::GetTrampoline()
#define GET_VALUE(RET,PTR) *std::bit_cast<RET*>(PTR)
#define ScopedEnum(N,T,...) struct N##Scope { enum N##Enum : T { __VA_ARGS__ }; }; using N = N##Scope::N##Enum
