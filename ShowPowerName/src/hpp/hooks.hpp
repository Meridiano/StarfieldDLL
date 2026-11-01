#pragma once
#include "hpp/dx12.hpp"

namespace SPN_Hooks {

	ScopedEnum(QueueMethods, std::uint32_t,
		QueryInterface = 0,
		AddRef,
		Release,
		GetPrivateData,
		SetPrivateData,
		SetPrivateDataInterface,
		SetName,
		GetDevice,
		UpdateTileMappings,
		CopyTileMappings,
		ExecuteCommandLists,
		SetMarker,
		BeginEvent,
		EndEvent,
		Signal,
		Wait,
		GetTimestampFrequency,
		GetClockCalibration,
		GetDesc,
		// end of table
		kTotal
	);
	static_assert(QueueMethods::kTotal == 19);

	ScopedEnum(SwapChainMethods, std::uint32_t,
		QueryInterface = 0,
		AddRef,
		Release,
		SetPrivateData,
		SetPrivateDataInterface,
		GetPrivateData,
		GetParent,
		GetDevice,
		Present,
		GetBuffer,
		SetFullscreenState,
		GetFullscreenState,
		GetDesc,
		ResizeBuffers,
		ResizeTarget,
		GetContainingOutput,
		GetFrameStatistics,
		GetLastPresentCount,
		// end of table
		kTotal
	);
	static_assert(SwapChainMethods::kTotal == 18);

	ScopedEnum(SwapChain1Methods, std::uint32_t,
		GetDesc1 = SwapChainMethods::kTotal,
		GetFullscreenDesc,
		GetIntPtr,
		GetCoreWindow,
		Present1,
		IsTemporaryMonoSupported,
		GetRestrictToOutput,
		SetBackgroundColor,
		GetBackgroundColor,
		SetRotation,
		GetRotation,
		// end of table
		kTotal
	);
	static_assert(SwapChain1Methods::kTotal == 29);

	struct ExecuteHook {
		static void NEW(ID3D12CommandQueue* queue, std::uint32_t num, ID3D12CommandList* const* lists) {
			if (queue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) SPN_DirectX::State::threadQueue = queue;
			OLD(queue, num, lists);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};

	struct PresentHook {
		static HRESULT NEW(IDXGISwapChain* swapChain, std::uint32_t syncInterval, std::uint32_t flags) {
			if (!(flags & DXGI_PRESENT_TEST)) SPN_DirectX::Render(swapChain);
			return OLD(swapChain, syncInterval, flags);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};

	struct Present1Hook {
		static HRESULT NEW(IDXGISwapChain1* swapChain, std::uint32_t syncInterval, std::uint32_t flags, const DXGI_PRESENT_PARAMETERS* params) {
			if (!(flags & DXGI_PRESENT_TEST)) SPN_DirectX::Render(swapChain);
			return OLD(swapChain, syncInterval, flags, params);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};

	struct InputEventHook {
		static void NEW(RE::BSInputEventSingleUser* singleUser, RE::InputEvent* queueHead) {
			if (SPN_DirectX::State::initDone) for (auto event = queueHead; event; event = event->next) {
				using IEType = RE::InputEvent::EventType;
				switch (event->eventType) {
					case IEType::kButton:
						SPN_ImGUI::Input::AddInput<IEType::kButton>(event);
						break;
					case IEType::kMouseMove:
						SPN_ImGUI::Input::AddInput<IEType::kMouseMove>(event);
						break;
					case IEType::kCursorMove:
						SPN_ImGUI::Input::AddInput<IEType::kCursorMove>(event);
						break;
					case IEType::kChar:
						SPN_ImGUI::Input::AddInput<IEType::kChar>(event);
						break;
					case IEType::kThumbstick:
						SPN_ImGUI::Input::AddInput<IEType::kThumbstick>(event);
						break;
				}
			}
			OLD(singleUser, queueHead);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};

	struct ScaleformRenderHook {
		static void NEW(SPN_Data::ScaleformRenderPass* renderPass, std::uintptr_t renderPassData) {
			static REL::Relocation<bool*> ready{ REL::ID(934195) };
			if (*ready && SPN_DirectX::State::initDone && renderPass) {
				if (auto name = renderPass->itemName; name && SPN_Utility::GetHudState(name)) {
					if (auto first = SPN_Utility::GetFirstMenuName(); first.compare(name) == 0) {
						if (auto cmdList = SPN_Utility::GetGraphicsCommandList(renderPassData); cmdList) {
							SPN_ImGUI::RenderLayer(SPN_ImGUI::targetLP, nullptr);
							SPN_ImGUI::RenderLayer(SPN_ImGUI::targetLP, cmdList);
						}
					}
				}
			}
			return OLD(renderPass, renderPassData);
		}
		static inline REL::Relocation<decltype(NEW)> OLD;
	};

	void Install() {
		auto [queueVTable, swapChainVTable] = SPN_Utility::GetVirtualTables();
		if (queueVTable && swapChainVTable) {
			ExecuteHook::OLD = REL::Relocation(queueVTable).write_vfunc(QueueMethods::ExecuteCommandLists, ExecuteHook::NEW);
			PresentHook::OLD = REL::Relocation(swapChainVTable).write_vfunc(SwapChainMethods::Present, PresentHook::NEW);
			Present1Hook::OLD = REL::Relocation(swapChainVTable).write_vfunc(SwapChain1Methods::Present1, Present1Hook::NEW);
			REX::INFO("DirectX hooks installed at {:X} / {:X}", queueVTable, swapChainVTable);
		} else REX::FAIL("DirectX tables not found");
		
		auto scaleformRenderVTable = REL::ID(497642).address();
		ScaleformRenderHook::OLD = REL::Relocation(scaleformRenderVTable).write_vfunc(0x7, ScaleformRenderHook::NEW);
		REX::INFO("Scaleform render hook installed at {:X}", scaleformRenderVTable);
		
		auto inputManagerVTable = REL::ID(469743).address();
		InputEventHook::OLD = REL::Relocation(inputManagerVTable).write_vfunc(0x1, InputEventHook::NEW);
		REX::INFO("Input hook installed at {:X}", inputManagerVTable);
	}

}
