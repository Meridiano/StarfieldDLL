#pragma once

namespace EZCUtility {

	RE::TESForm* GetFormFromFile(std::string a_name, std::uint32_t a_offset) {
		if (a_name.size() && a_offset) {
			auto sName = RE::BSFixedString(a_name);
			auto iOffset = std::int32_t(a_offset & 0xFFFFFF);
			using type = RE::TESForm*(*)(std::int64_t, std::int64_t, std::int64_t, std::int32_t, RE::BSFixedString*);
			static REL::Relocation<type> func{ REL::ID(117382) };
			return func(NULL, NULL, NULL, iOffset, &sName);
		}
		return nullptr;
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

	std::string CustomFormType(RE::TESForm* form) {
		if (form) switch (form->formType.underlying()) {
			case COBJ:
				return "COBJ";
			case RSPJ:
				return "RSPJ";
			default:
				return "FORM";
		}
		return "NONE";
	}

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		auto reloc = REL::Relocation<T*>(address);
		return reloc.get();
	};

}