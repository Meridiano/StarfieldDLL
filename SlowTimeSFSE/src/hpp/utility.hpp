#pragma once

namespace SlowTimeUtility {

	class GameTimeMultiplier {
	private:
		float global;
		float player;
		std::vector<float*> GetPointers() {
			// easy part
			static REL::Relocation<float*> glo_1{ REL::ID(763331) };
			static REL::Relocation<float*> glo_2{ REL::ID(763332) };
			static REL::Relocation<float*> glo_3{ REL::ID(891034), 0x18 };
			static REL::Relocation<float*> pla_1{ REL::ID(891034), 0x30 };
			// hard part 1
			static REL::Relocation<std::uintptr_t*> ptr_1{ REL::ID(891034), 0x10 };
			static REL::Relocation<float*> glo_4{ *ptr_1.get() + 0x4 };
			static REL::Relocation<float*> pla_2{ *ptr_1.get() + 0x8 };
			// hard part 2
			static REL::Relocation<std::uintptr_t*> ptr_2{ REL::ID(891034), 0x38 };
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
			static REL::Relocation<type> func{ REL::ID(172407) };
			func(NULL, NULL, owner, info, value);
		}
	};

	std::vector<std::string> Split(std::string input, char delim) {
		std::vector<std::string> result;
		std::stringstream stream(input);
		std::string item;
		while (std::getline(stream, item, delim)) result.push_back(item);
		return result;
	}

	template<typename T>
	auto ConvertTo(std::string raw) {
		auto StringToBool = [](std::string str) {
			switch (str.length()) {
				case 1:
					if (!str.compare("1")) return true;
					if (!str.compare("0")) return false;
					break;
				case 4:
					if (!strnicmp(str.data(), "true", 4)) return true;
					break;
				case 5:
					if (!strnicmp(str.data(), "false", 5)) return false;
					break;
			}
			throw std::exception("non-boolean string argument");
		};
		T val = T{};
		bool suc = true;
		while (suc) try {
	#define TRY_TYPE(TYPE, FUNC) if constexpr (std::is_same<T, TYPE>::value) { val = FUNC; break; }
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
		} catch (...) { suc = false; }
		return std::pair(suc, val);
	}

	RE::ActorValueInfo* LocateActorValue(std::string edid) {
		auto form = RE::TESForm::LookupByEditorID(edid);
		if (form && form->formType == RE::FormType::kAVIF) {
			return static_cast<RE::ActorValueInfo*>(form);
		}
		return nullptr;
	}

	bool CheckActorValue(RE::TESObjectREFR* avo, RE::ActorValueInfo* avi, bool mode, float low) {
		if (avo && avi) {
			auto avm = ActorValueManager(avo, avi);
			auto value = mode ? avm.GetPercent() : avm.Get();
			return (value > low);
		}
		return false;
	}

	void DamageActorValue(RE::TESObjectREFR* avo, RE::ActorValueInfo* avi, bool mode, float val) {
		if (avo && avi) {
			auto avm = ActorValueManager(avo, avi);
			if (mode) val *= avm.GetBase();
			return avm.Damage(val);
		}
	}

	bool IsDataLoaded() {
		static REL::Relocation<bool*> ptr{ REL::ID(881028) };
		return *ptr;
	}

	void PlayWAV(std::string path) {
		bool result = PlaySound(path.data(), NULL, SND_FILENAME + SND_ASYNC);
		if (!result) logs::info("Error on sound play, path = \"{}\"", path);
	}

	bool CheckModifiers(bool alt, bool ctrl, bool shift) {
		bool altVal = (GetAsyncKeyState(VK_MENU) & 0x8000);
		if (altVal != alt) return false;
		bool ctrlVal = (GetAsyncKeyState(VK_CONTROL) & 0x8000);
		if (ctrlVal != ctrl) return false;
		bool shiftVal = (GetAsyncKeyState(VK_SHIFT) & 0x8000);
		if (shiftVal != shift) return false;
		// all good
		return true;
	}

}