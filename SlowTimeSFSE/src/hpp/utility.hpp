#pragma once

namespace SlowTimeUtility {

	class GameTimeMultiplier {
	private:
		float global;
		float player;
		std::vector<float*> GetPointers() {
			// easy part
			static REL::Relocation<float*> glo_1{ REL::ID(810329) };
			static REL::Relocation<float*> glo_2{ REL::ID(810331) };
			static REL::Relocation<float*> glo_3{ REL::ID(949786), 0x20 };
			static REL::Relocation<float*> pla_1{ REL::ID(949786), 0x38 };
			// hard part 1
			static REL::Relocation<std::uintptr_t*> ptr_1{ REL::ID(949786), 0x18 };
			static REL::Relocation<float*> glo_4{ *ptr_1.get() + 0x4 };
			static REL::Relocation<float*> pla_2{ *ptr_1.get() + 0x8 };
			// hard part 2
			static REL::Relocation<std::uintptr_t*> ptr_2{ REL::ID(949786), 0x40 };
			static REL::Relocation<float*> pla_3{ *ptr_2.get() + 0x10 };
			static REL::Relocation<float*> glo_5{ *ptr_2.get() + 0x38 };
			// done
			return {
				glo_1.get(), glo_2.get(),
				glo_3.get(), pla_1.get(),
				glo_4.get(), pla_2.get(),
				pla_3.get(), glo_5.get()
			};
		}
	public:
		GameTimeMultiplier(float gloValue, float plaValue) {
			global = gloValue;
			player = plaValue;
		}
		void Apply() {
			auto list = GetPointers();
			*list[0] = *list[1] = *list[2] = *list[4] = *list[7] = global;
			*list[3] = *list[5] = *list[6] = player;
		}
	};

	class ActorValueManager {
	private:
		RE::TESObjectREFR* owner;
		RE::ActorValueInfo* info;
	public:
		ActorValueManager(RE::TESObjectREFR* a1, RE::ActorValueInfo* a2) {
			owner = a1;
			info = a2;
		}
		float GetPercentAgainst(ActorValueManager* arg) {
			return arg ? Get() / arg->GetBase() : 0.0F;
		}
		float GetPercent() {
			return Get() / GetBase();
		}
		float GetBase() {
			return owner->GetPermanentActorValue(*info);
		}
		float Get() {
			return owner->GetActorValue(*info);
		}
		void Damage(float value) {
			using type = void(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*, RE::ActorValueInfo*, float);
			static REL::Relocation<type> func{ REL::ID(118265) };
			func(NULL, NULL, owner, info, value);
		}
		void Restore(float value) {
			using type = void(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*, RE::ActorValueInfo*, float);
			static REL::Relocation<type> func{ REL::ID(118431) };
			func(NULL, NULL, owner, info, value);
		}
	};

	class ActorValueHelper {
	public:
		static RE::ActorValueInfo* Locate(std::string edid) {
			auto form = RE::TESForm::LookupByEditorID(edid);
			if (form && form->formType == RE::FormType::kAVIF) {
				return static_cast<RE::ActorValueInfo*>(form);
			}
			return nullptr;
		}
		static bool Check(RE::TESObjectREFR* avo, RE::ActorValueInfo* source, RE::ActorValueInfo* target, bool mode, bool invert, float threshold) {
			if (avo && source && target) {
				auto avmSource = ActorValueManager(avo, source);
				auto avmTarget = ActorValueManager(avo, target);
				auto value = mode ? avmSource.GetPercentAgainst(&avmTarget) : avmSource.Get();
				return invert ? (value < threshold) : (value > threshold);
			}
			return false;
		}
		static void Modify(RE::TESObjectREFR* avo, RE::ActorValueInfo* source, RE::ActorValueInfo* target, bool mode, float val) {
			if (avo && source && target) {
				auto avmSource = ActorValueManager(avo, source);
				auto avmTarget = ActorValueManager(avo, target);
				if (mode) val *= avmTarget.GetBase();
				if (val > 0.0F) avmSource.Damage(val);
				if (val < 0.0F) avmSource.Restore(val);
			}
		}
	};

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() && a_offset) {
			auto sName = RE::BSFixedString(a_name);
			auto iOffset = std::int32_t(a_offset & 0xFFFFFF);
			using type = RE::TESForm*(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type*> func{ REL::ID(117382) };
			return func(NULL, NULL, NULL, iOffset, &sName);
		}
		return nullptr;
	}

	std::vector<std::string> Split(std::string input, char delim) {
		std::vector<std::string> result;
		std::stringstream stream(input);
		std::string item;
		while (std::getline(stream, item, delim)) result.push_back(item);
		return result;
	}

	template<typename T>
	auto ConvertTo(std::string raw) {
		auto StringToBool = [](std::string text) {
			switch (text.length()) {
				case 1:
					if (!text.compare("1")) return true;
					if (!text.compare("0")) return false;
					break;
				case 4:
					if (!strnicmp(text.data(), "true", 4)) return true;
					break;
				case 5:
					if (!strnicmp(text.data(), "false", 5)) return false;
					break;
			}
			throw std::exception("non-boolean string argument");
		};
		T result{};
		bool success = true;
		while (success) try {
			#define TRY_TYPE(TYPE, FUNC) if constexpr (std::is_same<T, TYPE>::value) { result = FUNC; break; }
			TRY_TYPE(bool, StringToBool(raw));
			TRY_TYPE(std::int64_t, std::stoll(raw, nullptr, 0));
			TRY_TYPE(std::uint64_t, std::stoull(raw, nullptr, 0));
			TRY_TYPE(std::int32_t, std::stol(raw, nullptr, 0));
			TRY_TYPE(std::uint32_t, std::stoul(raw, nullptr, 0));
			TRY_TYPE(std::int16_t, std::stol(raw, nullptr, 0) & 0xFFFF);
			TRY_TYPE(std::uint16_t, std::stoul(raw, nullptr, 0) & 0xFFFF);
			TRY_TYPE(std::int8_t, std::stol(raw, nullptr, 0) & 0xFF);
			TRY_TYPE(std::uint8_t, std::stoul(raw, nullptr, 0) & 0xFF);
			TRY_TYPE(float, std::stof(raw, nullptr));
			TRY_TYPE(double, std::stod(raw, nullptr));
			TRY_TYPE(std::string, raw);
			#undef TRY_TYPE
			throw std::exception("unknown template type");
		} catch (...) { success = false; }
		return std::pair(success, result);
	}

	void DebugNotification(std::string message) {
		RE::BSFixedString fixedMessage = message;
		using type = void(*)(std::int64_t, std::int64_t, std::int64_t, RE::BSFixedString*);
		static REL::Relocation<type> function{ REL::ID(117311) };
		function(NULL, NULL, NULL, &fixedMessage);
	}

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		REL::Relocation<T*> reloc{ address };
		return reloc.get();
	}

	auto GetMenuEventSource(RE::UI* ui) {
		using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
		return GetMember<type>(ui, 0x20);
	}

	bool IsDataLoaded() {
		static REL::Relocation<bool*> ptr{ REL::ID(883582) };
		return *ptr;
	}

	bool GameIsFocused() {
		auto window = GetForegroundWindow();
		auto procID = GetCurrentProcessId();
		if (window && procID) {
			DWORD newProcID = NULL;
			auto thread = GetWindowThreadProcessId(window, &newProcID);
			if (thread && newProcID == procID) return (IsIconic(window) == 0);
		}
		return false;
	}

	bool ValidGamepadButton(std::int32_t button) {
		if (button < GLFW_GAMEPAD_BUTTON_A) return false;
		if (button > GLFW_GAMEPAD_BUTTON_LAST) return false;
		return true;
	}

	bool GamepadButtonPressed(std::int32_t button) {
		if (static bool ready = glfwInit(); ready) {
			glfwPollEvents();
			std::int32_t gamepadID = []() {
				static std::int32_t postLast = GLFW_JOYSTICK_LAST + 1;
				for (std::int32_t id = GLFW_JOYSTICK_1; id < postLast; id++)
					if (glfwJoystickIsGamepad(id))
						return id;
				return -1;
			}();
			if (gamepadID >= GLFW_JOYSTICK_1)
				if (GLFWgamepadstate state; glfwGetGamepadState(gamepadID, &state))
					return state.buttons[button] == GLFW_PRESS;
		} else REX::FAIL("Failed to initialize GLFW");
		return false;
	}

	bool HotkeyPressed(bool gamepad, std::int32_t value) {
		if (gamepad) return (ValidGamepadButton(value) ? GamepadButtonPressed(value) : false);
		return (GetAsyncKeyState(value) & 0x8000);
	}

	bool ModifierPressed(bool gamepad, std::int32_t value) {
		if (gamepad) return (ValidGamepadButton(value) ? GamepadButtonPressed(value) : true);
		// keyboard
		bool altState = (GetAsyncKeyState(VK_MENU) & 0x8000);
		if (static bool alt = (value & 1); altState != alt) return false;
		bool ctrlState = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
		if (static bool ctrl = (value & 2); ctrlState != ctrl) return false;
		bool shiftState = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
		if (static bool shift = (value & 4); shiftState != shift) return false;
		// all good
		return true;
	}

	class WaveAudioFile {
	public:
		enum Error : std::uint8_t {
			noError = 0,
			pathError,
			volumeError,
			streamError,
			headerError,
			loadError,
			playError
		};
	private:
		Error error = noError;
		float soundVolume = 0.0F;
		sf::SoundBuffer soundBuffer;
		std::string name = "Unknown";
	public:
		WaveAudioFile(fs::path filePath = "", float volume = 1.0F) {
			if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
				if (volume > 0.0F) {
					static auto streamFlags = std::ios::binary + std::ios::ate;
					if (std::ifstream file(filePath, streamFlags); file) {
						// read to buffer
						std::size_t size = file.tellg();
						file.seekg(NULL, std::ios::beg);
						std::vector<char> buffer(size);
						auto data = buffer.data();
						file.read(data, size);
						// is valid wav
						std::string riff(&buffer[0], 4);
						std::string wave(&buffer[8], 4);
						if (riff == "RIFF" && wave == "WAVE") {
							if (soundBuffer.loadFromMemory(data, size)) {
								soundVolume = volume;
							} else error = loadError;
						} else error = headerError;
					} else error = streamError;
				} else error = volumeError;
			} else error = pathError;
			auto newName = filePath.filename().string();
			if (newName.size() != 0) name = newName;
		}
		Error Play(std::optional<sf::Sound>& handle) {
			if (error != noError) return error;
			using SS = sf::SoundSource::Status;
			if (handle && handle->getStatus() == SS::Playing) handle->stop();
			handle.emplace(soundBuffer);
			handle->setVolume(soundVolume);
			handle->play();
			return (handle->getStatus() == SS::Stopped ? playError : noError);
		}
		std::string Filename() { return name; }
	};

	class ImageSpaceHelper {
	public:
		enum Error : std::uint8_t {
			noError = 0,
			formError,
			typeError,
			powerError
		};
	private:
		using VMDummy = std::int64_t;
		RE::TESImageSpaceModifier* imad = nullptr;
		float applyPower = 0.0F;
		Error error = noError;
	public:
		ImageSpaceHelper(std::string pluginName, std::uint32_t recordID, float power) {
			if (auto form = GetFormFromFile(pluginName, recordID); form) {
				if (form->formType == RE::FormType::kIMAD) {
					if (power > 0.0F) {
						imad = reinterpret_cast<decltype(imad)>(form);
						applyPower = power;
					} else error = powerError;
				} else error = typeError;
			} else error = formError;
		}
		Error Apply() {
			if (error) return error;
			using type = void(VMDummy, VMDummy, decltype(imad), decltype(applyPower));
			static REL::Relocation<type*> func{ REL::ID(118017) };
			func(NULL, NULL, imad, applyPower);
			return noError;
		}
		Error Remove() {
			if (error) return error;
			using type = void(VMDummy, VMDummy, decltype(imad));
			static REL::Relocation<type*> func{ REL::ID(118020) };
			func(NULL, NULL, imad);
			return noError;
		}
	};

}