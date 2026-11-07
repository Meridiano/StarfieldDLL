#pragma once

std::string tomlPath{ "" };
std::string fontPath{ "" };
std::string iguiPath{ "" };

namespace SPN_Data {

	std::unordered_map<std::int32_t, ImGuiKey> KeyboardMap = {
		{ VK_BACK, ImGuiKey::ImGuiKey_Backspace },
		{ VK_TAB, ImGuiKey::ImGuiKey_Tab },
		{ VK_RETURN, ImGuiKey::ImGuiKey_Enter },
		{ VK_PAUSE, ImGuiKey::ImGuiKey_Pause },
		{ VK_CAPITAL, ImGuiKey::ImGuiKey_CapsLock },
		{ VK_ESCAPE, ImGuiKey::ImGuiKey_Escape },
		{ VK_SPACE, ImGuiKey::ImGuiKey_Space },
		{ VK_PRIOR, ImGuiKey::ImGuiKey_PageUp },
		{ VK_NEXT, ImGuiKey::ImGuiKey_PageDown },
		{ VK_END, ImGuiKey::ImGuiKey_End },
		{ VK_HOME, ImGuiKey::ImGuiKey_Home },
		{ VK_LEFT, ImGuiKey::ImGuiKey_LeftArrow },
		{ VK_UP, ImGuiKey::ImGuiKey_UpArrow },
		{ VK_RIGHT, ImGuiKey::ImGuiKey_RightArrow },
		{ VK_DOWN, ImGuiKey::ImGuiKey_DownArrow },
		{ VK_SNAPSHOT, ImGuiKey::ImGuiKey_PrintScreen },
		{ VK_INSERT, ImGuiKey::ImGuiKey_Insert },
		{ VK_DELETE, ImGuiKey::ImGuiKey_Delete },
		{ 0x30, ImGuiKey::ImGuiKey_0 }, { 0x31, ImGuiKey::ImGuiKey_1 },
		{ 0x32, ImGuiKey::ImGuiKey_2 }, { 0x33, ImGuiKey::ImGuiKey_3 },
		{ 0x34, ImGuiKey::ImGuiKey_4 }, { 0x35, ImGuiKey::ImGuiKey_5 },
		{ 0x36, ImGuiKey::ImGuiKey_6 }, { 0x37, ImGuiKey::ImGuiKey_7 },
		{ 0x38, ImGuiKey::ImGuiKey_8 }, { 0x39, ImGuiKey::ImGuiKey_9 },
		{ 0x41, ImGuiKey::ImGuiKey_A }, { 0x42, ImGuiKey::ImGuiKey_B },
		{ 0x43, ImGuiKey::ImGuiKey_C }, { 0x44, ImGuiKey::ImGuiKey_D },
		{ 0x45, ImGuiKey::ImGuiKey_E }, { 0x46, ImGuiKey::ImGuiKey_F },
		{ 0x47, ImGuiKey::ImGuiKey_G }, { 0x48, ImGuiKey::ImGuiKey_H },
		{ 0x49, ImGuiKey::ImGuiKey_I }, { 0x4A, ImGuiKey::ImGuiKey_J },
		{ 0x4B, ImGuiKey::ImGuiKey_K }, { 0x4C, ImGuiKey::ImGuiKey_L },
		{ 0x4D, ImGuiKey::ImGuiKey_M }, { 0x4E, ImGuiKey::ImGuiKey_N },
		{ 0x4F, ImGuiKey::ImGuiKey_O }, { 0x50, ImGuiKey::ImGuiKey_P },
		{ 0x51, ImGuiKey::ImGuiKey_Q }, { 0x52, ImGuiKey::ImGuiKey_R },
		{ 0x53, ImGuiKey::ImGuiKey_S }, { 0x54, ImGuiKey::ImGuiKey_T },
		{ 0x55, ImGuiKey::ImGuiKey_U }, { 0x56, ImGuiKey::ImGuiKey_V },
		{ 0x57, ImGuiKey::ImGuiKey_W }, { 0x58, ImGuiKey::ImGuiKey_X },
		{ 0x59, ImGuiKey::ImGuiKey_Y }, { 0x5A, ImGuiKey::ImGuiKey_Z },
		{ VK_NUMPAD0, ImGuiKey::ImGuiKey_Keypad0 }, { VK_NUMPAD1, ImGuiKey::ImGuiKey_Keypad1 },
		{ VK_NUMPAD2, ImGuiKey::ImGuiKey_Keypad2 }, { VK_NUMPAD3, ImGuiKey::ImGuiKey_Keypad3 },
		{ VK_NUMPAD4, ImGuiKey::ImGuiKey_Keypad4 }, { VK_NUMPAD5, ImGuiKey::ImGuiKey_Keypad5 },
		{ VK_NUMPAD6, ImGuiKey::ImGuiKey_Keypad6 }, { VK_NUMPAD7, ImGuiKey::ImGuiKey_Keypad7 },
		{ VK_NUMPAD8, ImGuiKey::ImGuiKey_Keypad8 }, { VK_NUMPAD9, ImGuiKey::ImGuiKey_Keypad9 },
		{ VK_MULTIPLY, ImGuiKey::ImGuiKey_KeypadMultiply },
		{ VK_ADD, ImGuiKey::ImGuiKey_KeypadAdd },
		{ VK_SUBTRACT, ImGuiKey::ImGuiKey_KeypadSubtract },
		{ VK_DECIMAL, ImGuiKey::ImGuiKey_KeypadDecimal },
		{ VK_DIVIDE, ImGuiKey::ImGuiKey_KeypadDivide },
		{ VK_F1, ImGuiKey::ImGuiKey_F1 }, { VK_F2, ImGuiKey::ImGuiKey_F2 }, { VK_F3, ImGuiKey::ImGuiKey_F3 },
		{ VK_F4, ImGuiKey::ImGuiKey_F4 }, { VK_F5, ImGuiKey::ImGuiKey_F5 }, { VK_F6, ImGuiKey::ImGuiKey_F6 },
		{ VK_F7, ImGuiKey::ImGuiKey_F7 }, { VK_F8, ImGuiKey::ImGuiKey_F8 }, { VK_F9, ImGuiKey::ImGuiKey_F9 },
		{ VK_F10, ImGuiKey::ImGuiKey_F10 }, { VK_F11, ImGuiKey::ImGuiKey_F11 }, { VK_F12, ImGuiKey::ImGuiKey_F12 },
		{ VK_F13, ImGuiKey::ImGuiKey_F13 }, { VK_F14, ImGuiKey::ImGuiKey_F14 }, { VK_F15, ImGuiKey::ImGuiKey_F15 },
		{ VK_F16, ImGuiKey::ImGuiKey_F16 }, { VK_F17, ImGuiKey::ImGuiKey_F17 }, { VK_F18, ImGuiKey::ImGuiKey_F18 },
		{ VK_F19, ImGuiKey::ImGuiKey_F19 }, { VK_F20, ImGuiKey::ImGuiKey_F20 }, { VK_F21, ImGuiKey::ImGuiKey_F21 },
		{ VK_F22, ImGuiKey::ImGuiKey_F22 }, { VK_F23, ImGuiKey::ImGuiKey_F23 }, { VK_F24, ImGuiKey::ImGuiKey_F24 },
		{ VK_NUMLOCK, ImGuiKey::ImGuiKey_NumLock },
		{ VK_SCROLL, ImGuiKey::ImGuiKey_ScrollLock },
		{ VK_LSHIFT, ImGuiKey::ImGuiKey_LeftShift }, { VK_RSHIFT, ImGuiKey::ImGuiKey_RightShift },
		{ VK_LCONTROL, ImGuiKey::ImGuiKey_LeftCtrl }, { VK_RCONTROL, ImGuiKey::ImGuiKey_RightCtrl },
		{ VK_LMENU, ImGuiKey::ImGuiKey_LeftAlt }, { VK_RMENU, ImGuiKey::ImGuiKey_RightAlt },
		{ VK_LWIN, ImGuiKey::ImGuiKey_LeftSuper }, { VK_RWIN, ImGuiKey::ImGuiKey_RightSuper },
		{ VK_BROWSER_BACK, ImGuiKey::ImGuiKey_AppBack },
		{ VK_BROWSER_FORWARD, ImGuiKey::ImGuiKey_AppForward },
		{ VK_OEM_PLUS, ImGuiKey::ImGuiKey_Equal },
		{ VK_OEM_COMMA, ImGuiKey::ImGuiKey_Comma },
		{ VK_OEM_MINUS, ImGuiKey::ImGuiKey_Minus },
		{ VK_OEM_PERIOD, ImGuiKey::ImGuiKey_Period },
		{ VK_OEM_1, ImGuiKey::ImGuiKey_Semicolon },
		{ VK_OEM_2, ImGuiKey::ImGuiKey_Slash },
		{ VK_OEM_3, ImGuiKey::ImGuiKey_GraveAccent },
		{ VK_OEM_4, ImGuiKey::ImGuiKey_LeftBracket },
		{ VK_OEM_5, ImGuiKey::ImGuiKey_Backslash },
		{ VK_OEM_6, ImGuiKey::ImGuiKey_RightBracket },
		{ VK_OEM_7, ImGuiKey::ImGuiKey_Apostrophe },
		{ VK_OEM_102, ImGuiKey::ImGuiKey_Oem102 }
	};

	std::unordered_map<std::int32_t, ImGuiKey> MouseMap = {
		{ 0x0, ImGuiKey::ImGuiKey_MouseLeft },
		{ 0x1, ImGuiKey::ImGuiKey_MouseRight },
		{ 0x2, ImGuiKey::ImGuiKey_MouseMiddle },
		{ 0x3, ImGuiKey::ImGuiKey_MouseX1 },
		{ 0x4, ImGuiKey::ImGuiKey_MouseX2 },
		{ 0x800, ImGuiKey::ImGuiKey_MouseWheelX },
		{ 0x900, ImGuiKey::ImGuiKey_MouseWheelY }
	};

	std::unordered_map<std::int32_t, ImGuiKey> GamepadMap = {
		{ XINPUT_GAMEPAD_DPAD_UP, ImGuiKey_GamepadDpadUp },
		{ XINPUT_GAMEPAD_DPAD_DOWN, ImGuiKey_GamepadDpadDown },
		{ XINPUT_GAMEPAD_DPAD_LEFT, ImGuiKey_GamepadDpadLeft },
		{ XINPUT_GAMEPAD_DPAD_RIGHT, ImGuiKey_GamepadDpadRight },
		{ XINPUT_GAMEPAD_A, ImGuiKey_GamepadFaceDown },
		{ XINPUT_GAMEPAD_B, ImGuiKey_GamepadFaceRight },
		{ XINPUT_GAMEPAD_X, ImGuiKey_GamepadFaceLeft },
		{ XINPUT_GAMEPAD_Y, ImGuiKey_GamepadFaceUp },
		{ XINPUT_GAMEPAD_START, ImGuiKey_GamepadStart },
		{ XINPUT_GAMEPAD_BACK, ImGuiKey_GamepadBack },
		{ XINPUT_GAMEPAD_LEFT_SHOULDER, ImGuiKey_GamepadL1 },
		{ XINPUT_GAMEPAD_RIGHT_SHOULDER, ImGuiKey_GamepadR1 },
		{ 0x9, ImGuiKey_GamepadL2 }, { 0xA, ImGuiKey_GamepadR2 }
	};

	std::unordered_set<std::string> BlockModes = {
		"CharGen_Init",
		"CharGen_PreWatch",
		"ContainerMode",
		"DialogueCameraMode",
		"DialogueListMode",
		"DialogueMode",
		"FurnitureEnterExit",
		"GravJump",
		"InspectMode",
		"Invalid",
		"LockpickingMode",
		"MessageMode",
		"MovementDisabled",
		"None",
		"PhotoMode",
		"SecurityMenu",
		"SleepWaitMode",
		"SpaceshipHudMode",
		"SpaceshipLandingMode",
		"SpaceshipTakeoffMode",
		"TerminalMode",
		"WorkshopMode"
	};

	class MouseMoveEvent : public RE::IDEvent {
	public:
		virtual ~MouseMoveEvent();
		// members
		std::int32_t deltaX;
		std::int32_t deltaY;
	};
	static_assert(sizeof(MouseMoveEvent) == 0x40);

	class CursorMoveEvent : public RE::IDEvent {
	public:
		virtual ~CursorMoveEvent();
		// members
		std::int32_t posX;
		std::int32_t posY;
	};
	static_assert(sizeof(CursorMoveEvent) == 0x40);

	class CharacterEvent : public RE::InputEvent {
	public:
		virtual ~CharacterEvent();
		// members
		std::uint32_t charCode; // UTF-16 LE
		std::byte padding[4];
	};
	static_assert(sizeof(CharacterEvent) == 0x30);

	class ThumbstickEvent : public RE::IDEvent {
	public:
		virtual ~ThumbstickEvent();
		// members
		float xValue;
		float yValue;
		std::uint32_t flags;
		std::byte padding[4];
	};
	static_assert(sizeof(ThumbstickEvent) == 0x48);

	struct ScaleformRenderPass {
		void** virtualTable;
		LPCSTR itemName;
	};
	// probably incomplete
	static_assert(offsetof(ScaleformRenderPass, itemName) == 0x8);

	struct GraphicsCommandListHolder {
		std::byte unknownData[0x60];
		ID3D12GraphicsCommandList* cmdList;
	};
	// probably incomplete
	static_assert(offsetof(GraphicsCommandListHolder, cmdList) == 0x60);

}
