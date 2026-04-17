#pragma once

#include "SFSE/SFSE.h"
#include "RE/Starfield.h"

namespace fs = std::filesystem;
using namespace std::literals;

#include "../lib/mini/ini.h"

static const std::uint8_t COBJ = 0x97;
static const std::uint8_t RSPJ = 0xC1;
static const std::uint8_t LGDI = 0xCF;

#define ThreadID std::bit_cast<std::uint32_t>(std::this_thread::get_id())
#define SleepFor(MS) std::this_thread::sleep_for(std::chrono::milliseconds(MS))

using Component = RE::BSTTuple3<RE::TESForm*, RE::BGSCurveForm*, RE::BGSTypedFormValuePair::SharedVal>;
using ComponentList = RE::BSTArray<Component>;