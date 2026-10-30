#pragma once
#include "hpp/data.hpp"

namespace SPN_Utility {

	RE::SpellItem* GetEquippedSpell(RE::Actor* actor, std::uint32_t type) {
		using func_t = RE::SpellItem*(*)(std::uint64_t, std::uint32_t, RE::Actor*, std::uint32_t);
		static REL::Relocation<func_t> func{ REL::ID(116694) };
		return (actor && type < 4) ? func(NULL, NULL, actor, type) : nullptr;
	}

	std::string GetSpellName() {
		if (static REL::Relocation<bool*> dataLoaded{ REL::ID(883582) }; *dataLoaded) {
			if (auto player = RE::PlayerCharacter::GetSingleton(); player) {
				for (std::uint32_t type = 0; type < 4; type++) {
					if (auto spell = GetEquippedSpell(player, type); spell) {
						if (auto name = spell->GetFullName(); name && name[0]) {
							return std::string(name);
						}
					}
				}
			}
		}
		return std::string("");
	}

	ImGuiKey FindImGuiKey(std::int32_t dxCode, RE::InputEvent::DeviceType device) {
		auto result = ImGuiKey::ImGuiKey_None;
		switch (device) {
			case RE::InputEvent::DeviceType::kKeyboard: {
				static auto end = SPN_Data::KeyboardMap.end();
				auto entry = SPN_Data::KeyboardMap.find(dxCode);
				if (entry != end) result = entry->second;
			}   break;
			case RE::InputEvent::DeviceType::kMouse: {
				static auto end = SPN_Data::MouseMap.end();
				auto entry = SPN_Data::MouseMap.find(dxCode);
				if (entry != end) result = entry->second;
			}   break;
			case RE::InputEvent::DeviceType::kGamepad: {
				static auto end = SPN_Data::GamepadMap.end();
				auto entry = SPN_Data::GamepadMap.find(dxCode);
				if (entry != end) result = entry->second;
			}   break;
		}
		return result;
	}

	ID3D12GraphicsCommandList* GetGraphicsCommandList(std::uintptr_t renderPassData) {
		if (renderPassData) {
			using type = SPN_Data::GraphicsCommandListHolder*(*)(std::uintptr_t);
			static REL::Relocation<type> function{ REL::ID(144161) };
			auto result = function(renderPassData);
			if (result) return result->cmdList;
		}
		return nullptr;
	}

	std::string GetFirstMenuName() {
		using ArrayType = RE::BSTArray<RE::IMenu*>;
		static auto& menus = *[]() {
			auto ui = (std::uintptr_t)RE::UI::GetSingleton();
			REL::Relocation<ArrayType*> result{ ui + 0x430 };
			return result.get();
		}();
		if (menus.size() != 0)
			if (auto first = menus[0]; first)
				if (auto name = first->GetName(); name)
					return std::string(name);
		return "";
	}

	std::pair<std::uintptr_t, std::uintptr_t> GetVirtualTables() {
		std::pair<std::uintptr_t, std::uintptr_t> result{ 0x0, 0x0 };
		auto hwnd = CreateWindowA("STATIC", "Dummy", WS_OVERLAPPED, 0, 0, 100, 100, 0, 0, 0, 0);
		{
			// smarts
			ComPtr<ID3D12Device> device;
			ComPtr<ID3D12CommandQueue> queue;
			ComPtr<IDXGIFactory4> factory;
			ComPtr<IDXGISwapChain1> swapChain;
			// setup
			D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			D3D12_COMMAND_QUEUE_DESC queueDesc = {};
			device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue));
			CreateDXGIFactory1(IID_PPV_ARGS(&factory));
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			factory->CreateSwapChainForHwnd(queue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
			// tables
			result.first = *(std::uintptr_t*)queue.Get();
			result.second = *(std::uintptr_t*)swapChain.Get();
		} // destroy smarts here
		DestroyWindow(hwnd);
		return result;
	}

	void SetupStrings() {
		std::string plugin{ SFSE::GetPluginName() };
		auto root = std::format("Data\\SFSE\\Plugins\\{}", plugin);
		tomlPath = root + "\\Config.toml";
		iguiPath = root + "\\ImGUI.ini";
		fontPath = root + "\\Font.ttf";
	}

}
