#pragma once

namespace UCUtility {

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

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		auto reloc = REL::Relocation<T*>(address);
		return reloc.get();
	}

	auto GetMenuEventSource(RE::UI* ui) {
		using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
		return GetMember<type>(ui, 0x20);
	}

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

}