#pragma once

#define NOMINMAX
#include "RE/Starfield.h"
#include "SFSE/SFSE.h"

// dx12
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <dxgi1_4.h>
#pragma comment(lib, "dxgi.lib")

// microslop
#include <xinput.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// imgui
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

// toml
#include "toml++/toml.h"

// custom
#include <numbers>
#define SleepFor(N,T) std::this_thread::sleep_for(std::chrono::T(N))
#define ScopedEnum(N,T,...) struct N##Scope { enum N##Enum : T { __VA_ARGS__ }; }; using N = N##Scope::N##Enum
