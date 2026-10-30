#pragma once
#include "hpp/imgui.hpp"

namespace SPN_DirectX {

	namespace State {

		ComPtr<ID3D12Device> device;
		ComPtr<ID3D12CommandQueue> queue;
		thread_local ID3D12CommandQueue* threadQueue;
		ComPtr<ID3D12DescriptorHeap> srvHeap, rtvHeap;
		ComPtr<ID3D12CommandAllocator> allocator;
		ComPtr<ID3D12GraphicsCommandList> list;
		ComPtr<ID3D12Fence> fence;

		IDXGISwapChain* swapChain;
		DXGI_FORMAT format;
		WNDPROC wndProc;

		std::uint64_t fenceValue, srvNext;
		std::uint32_t bufferCount;
		bool initDone, busy;

	}

	namespace Memory {

		void SrvAlloc(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* cpu, D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
			auto inc = State::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			*cpu = State::srvHeap->GetCPUDescriptorHandleForHeapStart();
			*gpu = State::srvHeap->GetGPUDescriptorHandleForHeapStart();
			cpu->ptr += inc * State::srvNext;
			gpu->ptr += inc * State::srvNext++;
		}

		void SrvFree(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
			// do nothing because i'm lazy
		}

	}

	void WaitGpu() {
		static auto event = CreateEventW(0, 0, 0, 0);
		State::queue->Signal(State::fence.Get(), State::fenceValue += 1);
		State::fence->SetEventOnCompletion(State::fenceValue, event);
		WaitForSingleObject(event, INFINITE);
	}

	LRESULT WindowProcedure(HWND hwnd, std::uint32_t msg, WPARAM wParam, LPARAM lParam) {
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam) ? 1 : CallWindowProcW(State::wndProc, hwnd, msg, wParam, lParam);
	}

	void Setup(IDXGISwapChain3* swapChain, DXGI_SWAP_CHAIN_DESC* desc) {
		if (State::initDone) {
			State::initDone = false;
			WaitGpu();
			SPN_ImGUI::ShutdownLayer(SPN_ImGUI::configLP);
			SPN_ImGUI::ShutdownLayer(SPN_ImGUI::targetLP);
		}
		// reset
		State::wndProc = (WNDPROC)SetWindowLongPtrW(desc->OutputWindow, GWLP_WNDPROC, (LONG_PTR)WindowProcedure);
		State::queue = State::threadQueue;
		State::device.Reset();
		// setup
		swapChain->GetDevice(IID_PPV_ARGS(&State::device));
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 64, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE };
		State::device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&State::srvHeap));
		heapDesc = { D3D12_DESCRIPTOR_HEAP_TYPE_RTV, desc->BufferCount, D3D12_DESCRIPTOR_HEAP_FLAG_NONE };
		State::device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&State::rtvHeap));
		State::device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&State::allocator));
		State::device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, State::allocator.Get(), nullptr, IID_PPV_ARGS(&State::list));
		State::list->Close();
		State::device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&State::fence));
		State::fenceValue = State::srvNext = 0;
		// imgui
		ImGui_ImplDX12_InitInfo info = {};
		info.Device = State::device.Get();
		info.CommandQueue = State::queue.Get();
		info.NumFramesInFlight = desc->BufferCount;
		info.RTVFormat = desc->BufferDesc.Format;
		info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		info.SrvDescriptorHeap = State::srvHeap.Get();
		info.SrvDescriptorAllocFn = Memory::SrvAlloc;
		info.SrvDescriptorFreeFn = Memory::SrvFree;
		SPN_ImGUI::InitLayer(SPN_ImGUI::configLP, desc->OutputWindow, SPN_ImGUI::Window::Config, &info);
		SPN_ImGUI::InitLayer(SPN_ImGUI::targetLP, desc->OutputWindow, SPN_ImGUI::Window::Target, &info);
		// done
		State::swapChain = swapChain;
		State::format = desc->BufferDesc.Format;
		State::bufferCount = desc->BufferCount;
		State::initDone = true;
	}

	void Render(IDXGISwapChain* swapChainBase) {
		if (State::busy || !State::threadQueue) return;
		ComPtr<IDXGISwapChain3> swapChain;
		if (FAILED(swapChainBase->QueryInterface(IID_PPV_ARGS(&swapChain)))) return;
		ComPtr<ID3D12Device> device;
		if (FAILED(swapChain->GetDevice(IID_PPV_ARGS(&device)))) return;
		// start
		State::busy = true;
		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		bool needSetup = [&]() {
			if (!State::initDone) return true;
			if (swapChainBase != State::swapChain) return true;
			if (State::threadQueue != State::queue.Get()) return true;
			if (desc.BufferDesc.Format != State::format) return true;
			if (desc.BufferCount != State::bufferCount) return true;
			return false;
		}();
		if (needSetup) Setup(swapChain.Get(), &desc);
		// make frame
		ComPtr<ID3D12Resource> backBuffer;
		auto index = swapChain->GetCurrentBackBufferIndex();
		swapChain->GetBuffer(index, IID_PPV_ARGS(&backBuffer));
		auto rtv = State::rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtv.ptr += index * State::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		State::device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv);
		SPN_ImGUI::RenderLayer(SPN_ImGUI::configLP, nullptr);
		// make commands and finalize frame
		State::allocator->Reset();
		State::list->Reset(State::allocator.Get(), nullptr);
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition = { backBuffer.Get(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET };
		State::list->ResourceBarrier(1, &barrier);
		State::list->OMSetRenderTargets(1, &rtv, false, nullptr);
		ID3D12DescriptorHeap* heaps[] = { State::srvHeap.Get() };
		State::list->SetDescriptorHeaps(1, heaps);
		SPN_ImGUI::RenderLayer(SPN_ImGUI::configLP, State::list.Get());
		// execute commands
		std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
		State::list->ResourceBarrier(1, &barrier);
		State::list->Close();
		ID3D12CommandList* lists[] = { State::list.Get() };
		State::queue->ExecuteCommandLists(1, lists);
		// done
		WaitGpu();
		State::busy = false;
	}

}
