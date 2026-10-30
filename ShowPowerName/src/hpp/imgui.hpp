#pragma once
#include "hpp/config.hpp"

namespace SPN_ImGUI {

	struct UILayer {
		HWND hwnd = nullptr;
		std::function<void()> build;
		ImGuiContext* context = nullptr;
	};

	static UILayer configLayer, targetLayer;
	static UILayer* configLP = &configLayer;
	static UILayer* targetLP = &targetLayer;

	namespace Const {

		// basic
		constexpr float zero = 0.0F;
		constexpr float half = 0.5F;
		constexpr float one  = 1.0F;
		// gui
		constexpr float inputWidth  = 100.0F;
		constexpr float buttonWidth = inputWidth * half;
		constexpr float midWheel = float(ImGuiKey_MouseWheelX + ImGuiKey_MouseWheelY) * half;
		// font
		constexpr float maxFontSize = 512.0F;
		// degrees
		constexpr float deg180  = 180.0F;
		constexpr float deg90   = deg180 * half;
		constexpr float pi      = std::numbers::pi;
		constexpr float deg2Rad = pi / deg180;
		// distance
		constexpr float minDistance = 200.0F;
		constexpr float maxDistance = 5000.0F;
		// integers
		constexpr std::int32_t numStyles  = 2;
		constexpr std::int32_t anchorGrid = 3;
		constexpr std::int32_t maxSteps   = 16;
		// vectors
		constexpr ImVec2 zeroXY     = { zero, zero };
		constexpr ImVec2 buttonSize = { buttonWidth, zero };

	}

	bool visible = false;
	bool keyListener = true;
	std::mutex contextMutex;
	ImGuiIO* configIO = nullptr;
	ImVec2 mousePosition = Const::zeroXY;

	namespace Window {

		#define SameLine ImGui::SameLine()
		#define InputLine SameLine; ImGui::SetNextItemWidth(Const::inputWidth)

		void Config() {
			ImGui::Begin("Show Power Name");
			ImGui::ColorEdit3("Color", &SPN_Config::color[0]);
			ImGui::SliderFloat("##Opacity", &SPN_Config::alpha, Const::zero, Const::one);
			InputLine; ImGui::InputFloat("Opacity", &SPN_Config::alpha);
			ImGui::SliderFloat("##Size", &SPN_Config::fontSize, Const::one, Const::maxFontSize);
			InputLine; ImGui::InputFloat("Size", &SPN_Config::fontSize);
			ImGui::TextUnformatted("Anchor");
			for (std::int32_t row = 0; row < Const::anchorGrid; row++) {
				for (std::int32_t column = 0; column < Const::anchorGrid; column++) {
					auto index = row * Const::anchorGrid + column;
					if (column != 0) SameLine;
					ImGui::PushID(index);
					ImGui::RadioButton("##Anchor", &SPN_Config::anchor, index);
					ImGui::PopID();
				}
			}
			auto clipRect = ImGui::GetMainViewport()->WorkSize;
			ImGui::SliderFloat("##PosX", &SPN_Config::position.x, Const::zero, clipRect.x);
			InputLine; ImGui::InputFloat("Pos X", &SPN_Config::position.x);
			ImGui::SliderFloat("##PosY", &SPN_Config::position.y, Const::zero, clipRect.y);
			InputLine; ImGui::InputFloat("Pos Y", &SPN_Config::position.y);
			ImGui::SliderFloat("##Rotation", &SPN_Config::rotation, -Const::deg180, Const::deg180, "%.1f deg");
			InputLine; ImGui::InputFloat("Rotation", &SPN_Config::rotation);
			ImGui::SliderFloat("##TiltX", &SPN_Config::pitch, -Const::deg90, Const::deg90, "%.1f deg");
			InputLine; ImGui::InputFloat("Tilt X (pitch)", &SPN_Config::pitch);
			ImGui::SliderFloat("##TiltY", &SPN_Config::yaw, -Const::deg90, Const::deg90, "%.1f deg");
			InputLine; ImGui::InputFloat("Tilt Y (yaw)", &SPN_Config::yaw);
			ImGui::SliderFloat("##Perspective", &SPN_Config::distance, Const::minDistance, Const::maxDistance, "%.0f px");
			InputLine; ImGui::InputFloat("Perspective", &SPN_Config::distance);
			if (ImGui::Button("Save", Const::buttonSize)) SPN_Config::Save(false);
			SameLine; if (ImGui::Button("Load", Const::buttonSize)) SPN_Config::Load(false);
			SameLine; if (ImGui::Button("Reset", Const::buttonSize)) SPN_Config::Reset(false);
			ImGui::End();
		}

		#undef SameLine
		#undef InputLine

		#define SuperVectors(V1,V2) ImVec2 V1(FLT_MAX, FLT_MAX), V2(-FLT_MAX, -FLT_MAX)
		#define MinVector(V,T) V.x = fminf(V.x, T.x); V.y = fminf(V.y, T.y)
		#define MaxVector(V,T) V.x = fmaxf(V.x, T.x); V.y = fmaxf(V.y, T.y)
		#define MinMaxVectors(V1,V2,T) MinVector(V1, T); MaxVector(V2, T)

		void Target() {
			auto newText = SPN_Utility::GetSpellName();
			if (newText.size() == 0) return;
			LPCSTR newData = newText.data();
			// let's gooo
			auto clipRect = ImGui::GetMainViewport()->WorkSize;
			ImGui::SetNextWindowPos(Const::zeroXY);
			ImGui::SetNextWindowSize(clipRect);
			static auto flags = []() {
				ImGuiWindowFlags result = ImGuiWindowFlags_None;
				std::vector<ImGuiWindowFlags> list = {
					ImGuiWindowFlags_NoDecoration,
					ImGuiWindowFlags_NoBackground,
					ImGuiWindowFlags_NoInputs,
					ImGuiWindowFlags_NoMove,
					ImGuiWindowFlags_NoSavedSettings,
					ImGuiWindowFlags_NoFocusOnAppearing,
					ImGuiWindowFlags_NoBringToFrontOnFocus
				};
				for (auto flag : list) result |= flag;
				return result;
			}();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Const::zeroXY);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, Const::zero);
			ImGui::Begin("Target", nullptr, flags);
			ImGui::PopStyleVar(Const::numStyles);
			ImGui::BringWindowToDisplayBack(ImGui::GetCurrentWindow());
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pivot = SPN_Config::position;
			auto anchorX = float(SPN_Config::anchor % Const::anchorGrid) * Const::half;
			auto anchorY = float(SPN_Config::anchor / Const::anchorGrid) * Const::half;
			float cr = cosf(SPN_Config::rotation * Const::deg2Rad), sr = sinf(SPN_Config::rotation * Const::deg2Rad);
			float cp = cosf(SPN_Config::pitch * Const::deg2Rad), sp = sinf(SPN_Config::pitch * Const::deg2Rad);
			float cy = cosf(SPN_Config::yaw * Const::deg2Rad), sy = sinf(SPN_Config::yaw * Const::deg2Rad);
			auto Project = [&](float x0, float y0) {
				float x1 = x0 * cr - y0 * sr, y1 = x0 * sr + y0 * cr;  // roll
				float x2 = x1 * cy, z2 = x1 * sy;                      // yaw
				float y3 = y1 * cp + z2 * sp, z3 = -y1 * sp + z2 * cp; // pitch
				float scale = SPN_Config::distance / fmaxf(SPN_Config::distance + z3, Const::one);
				return ImVec2(x2 * scale, y3 * scale);
			};
			ImVec2 maxSize = SPN_Config::targetFont->CalcTextSizeA(Const::maxFontSize, FLT_MAX, Const::zero, newData);
			auto Fits = [&](float size) {
				float temp = size / Const::maxFontSize;
				float width = maxSize.x * temp, height = maxSize.y * temp;
				float left = -width * anchorX, right = width * (Const::one - anchorX);
				float top = -height * anchorY, bottom = height * (Const::one - anchorY);
				ImVec2 corners[4] = { { left, top }, { right, top }, { right, bottom }, { left, bottom } };
				SuperVectors(minBox, maxBox);
				for (const ImVec2& corner : corners) {
					ImVec2 p = Project(corner.x, corner.y);
					MinMaxVectors(minBox, maxBox, p);
				}
				bool resultX = (maxBox.x - minBox.x) <= clipRect.x;
				bool resultY = (maxBox.y - minBox.y) <= clipRect.y;
				return (resultX && resultY);
			};
			float localSize = SPN_Config::fontSize;
			if (!Fits(localSize)) {
				float low = Const::one, high = localSize;
				for (std::int32_t index = 0; index < Const::maxSteps; index++) {
					float mid = (low + high) * Const::half;
					(Fits(mid) ? low : high) = mid;
				}
				localSize = low;
			}
			ImVec2 calcSize = {
				maxSize.x * localSize / Const::maxFontSize,
				maxSize.y * localSize / Const::maxFontSize
			};
			float padding = calcSize.x + calcSize.y;
			auto vertexStart = drawList->VtxBuffer.Size;
			drawList->PushClipRect(
				ImVec2(-padding, -padding),
				ImVec2(clipRect.x + padding, clipRect.y + padding),
				false
			);
			drawList->AddText(
				SPN_Config::targetFont, localSize,
				ImVec2(pivot.x - calcSize.x * anchorX, pivot.y - calcSize.y * anchorY),
				ImGui::GetColorU32(ImVec4(SPN_Config::color.red, SPN_Config::color.green, SPN_Config::color.blue, SPN_Config::alpha)), newData
			);
			drawList->PopClipRect();
			auto vertexEnd = drawList->VtxBuffer.Size;
			SuperVectors(boundsMin, boundsMax);
			for (auto index = vertexStart; index < vertexEnd; index++) {
				ImDrawVert& vertex = drawList->VtxBuffer[index];
				ImVec2 p = Project(vertex.pos.x - pivot.x, vertex.pos.y - pivot.y);
				vertex.pos.x = pivot.x + p.x;
				vertex.pos.y = pivot.y + p.y;
				MinMaxVectors(boundsMin, boundsMax, vertex.pos);
			}
			if (vertexEnd > vertexStart) {
				auto Difference = [&](float low, float high, float area) {
					if (high - low >= area) return (area * Const::half - (low + high) * Const::half);
					if (low < Const::zero) return fabsf(low);
					if (high > area) return (area - high);
					return Const::zero;
				};
				float dx = Difference(boundsMin.x, boundsMax.x, clipRect.x);
				float dy = Difference(boundsMin.y, boundsMax.y, clipRect.y);
				if (dx != Const::zero || dy != Const::zero) for (auto index = vertexStart; index < vertexEnd; index++) {
					ImDrawVert& vertex = drawList->VtxBuffer[index];
					vertex.pos.x += dx;
					vertex.pos.y += dy;
				}
			}
			ImGui::End();
		}

		#undef SuperVectors
		#undef MinVector
		#undef MaxVector
		#undef MinMaxVectors

	}

	#define SafeSetCurrentContext(LP) std::lock_guard contextGuard(contextMutex); ImGui::SetCurrentContext(LP->context)

	namespace Input {

		void AdjustMousePosition(bool push, bool absolute = false, float x = Const::zero, float y = Const::zero) {
			if (push) {
				if (absolute) {
					mousePosition.x = x;
					mousePosition.y = y;
				} else {
					mousePosition.x += x;
					mousePosition.y += y;
				}
				auto clipRect = ImGui::GetMainViewport()->WorkSize;
				if (mousePosition.x < Const::zero) mousePosition.x = Const::zero;
				if (mousePosition.y < Const::zero) mousePosition.y = Const::zero;
				if (mousePosition.x > clipRect.x) mousePosition.x = clipRect.x;
				if (mousePosition.y > clipRect.y) mousePosition.y = clipRect.y;
			} else {
				configIO->MousePos = mousePosition;
				configIO->WantSetMousePos = true;
			}
		}

		template <RE::InputEvent::EventType type>
		void AddInput(RE::InputEvent* event) {
			SafeSetCurrentContext(configLP);
			if constexpr (type == RE::InputEvent::EventType::kButton) {
				auto button = static_cast<RE::ButtonEvent*>(event);
				auto keyCode = SPN_Utility::FindImGuiKey(button->idCode, button->deviceType);
				if (keyCode == ImGuiKey_None) return;
				bool keyDown = button->value;
				bool toggle = false;
				if (keyCode == SPN_Config::keyCode) {
					if (keyDown) {
						if (keyListener) {
							keyListener = false;
							visible = !visible;
							REX::INFO("{} config menu", visible ? "Show" : "Hide");
							toggle = true;
						}
					} else keyListener = true;
				}
				if (toggle || visible) {
					button->strUserEvent = "";
					button->idCode = -1;
				}
				if (toggle) return;
				if (visible) switch (keyCode) {
					case ImGuiKey_MouseWheelX:
					case ImGuiKey_MouseWheelY:
						configIO->AddMouseWheelEvent(Const::zero, Const::midWheel - (float)keyCode);
						break;
					case ImGuiKey_GamepadL2:
					case ImGuiKey_GamepadR2:
						configIO->AddKeyAnalogEvent(keyCode, keyDown, Const::half);
						break;
					default:
						configIO->AddKeyEvent(keyCode, keyDown);
						break;
				}
			}
			if constexpr (type == RE::InputEvent::EventType::kMouseMove) {
				auto mouse = reinterpret_cast<SPN_Data::MouseMoveEvent*>(event);
				if (visible) {
					AdjustMousePosition(true, false, mouse->deltaX, mouse->deltaY);
					mouse->deltaX = mouse->deltaY = 0;
				}
			}
			if constexpr (type == RE::InputEvent::EventType::kCursorMove) {
				auto cursor = reinterpret_cast<SPN_Data::CursorMoveEvent*>(event);
				static std::int32_t savedX = 0;
				static std::int32_t savedY = 0;
				if (visible) {
					cursor->posX = savedX;
					cursor->posY = savedY;
				} else {
					savedX = cursor->posX;
					savedY = cursor->posY;
					AdjustMousePosition(true, true, savedX, savedY);
				}
			}
			if constexpr (type == RE::InputEvent::EventType::kChar) {
				auto symbol = reinterpret_cast<SPN_Data::CharacterEvent*>(event);
				if (visible) symbol->charCode = 0;
			}
			if constexpr (type == RE::InputEvent::EventType::kThumbstick) {
				auto stick = reinterpret_cast<SPN_Data::ThumbstickEvent*>(event);
				if (visible) {
					bool left = (stick->xValue < Const::zero);
					bool down = (stick->yValue < Const::zero);
					float xAbs = fabsf(stick->xValue);
					float yAbs = fabsf(stick->yValue);
					switch (stick->idCode) {
						case 0xB:
							configIO->AddKeyAnalogEvent(left ? ImGuiKey_GamepadLStickLeft : ImGuiKey_GamepadLStickRight, true, xAbs);
							configIO->AddKeyAnalogEvent(down ? ImGuiKey_GamepadLStickDown : ImGuiKey_GamepadLStickUp, true, yAbs);
						case 0xC:
							configIO->AddKeyAnalogEvent(left ? ImGuiKey_GamepadRStickLeft : ImGuiKey_GamepadRStickRight, true, xAbs);
							configIO->AddKeyAnalogEvent(down ? ImGuiKey_GamepadRStickDown : ImGuiKey_GamepadRStickUp, true, yAbs);
					}
					stick->strUserEvent = "";
					stick->idCode = -1;
				}
			}
		}

	}

	void InitLayer(UILayer* layer, HWND hwnd, std::function<void()> build, ImGui_ImplDX12_InitInfo* info) {
		layer->hwnd = hwnd;
		layer->build = std::move(build);
		layer->context = ImGui::CreateContext();
		SafeSetCurrentContext(layer);
		// configure
		auto io = &ImGui::GetIO();
		if (layer == configLP) {
			io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io->IniFilename = iguiPath.data();
			io->Fonts->AddFontDefault();
			configIO = io;
		} else if (layer == targetLP) {
			io->IniFilename = nullptr;
			auto newFont = io->Fonts->AddFontFromFileTTF(fontPath.data(), Const::maxFontSize);
			if (!newFont) REX::FAIL("Failed to load font file");
			SPN_Config::targetFont = newFont;
		}
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX12_Init(info);
	}

	void RenderLayer(UILayer* layer, ID3D12GraphicsCommandList* cmdList) {
		SafeSetCurrentContext(layer);
		if (!cmdList) {
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			if (layer == configLP) {
				configIO->MouseDrawCursor = visible;
				if (visible) {
					Input::AdjustMousePosition(false);
					Window::Config();
				}
			} else if (layer == targetLP) Window::Target();
			ImGui::Render();
			return;
		}
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList);
	}

	void ShutdownLayer(UILayer* layer) {
		SafeSetCurrentContext(layer);
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext(layer->context);
		layer->context = nullptr;
	}

	#undef SafeSetCurrentContext

}
