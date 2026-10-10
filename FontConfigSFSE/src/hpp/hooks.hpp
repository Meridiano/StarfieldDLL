#pragma once

#include "hpp/process.hpp"

namespace FCHooks {

	struct ParseConfigHook {
		static inline std::string name{ "1AF0D603" };
		static void Modified(FCData::BSScaleformManager* scaleform) {
			Original(scaleform);
			REX::INFO("{} triggered, BSScaleformManager = {:X}", name, (std::uint64_t)scaleform);
			if (scaleform) FCProcess::ExtendedProcess(scaleform, REL::ID(130700));
		}
		inline static REL::THook Original{ REL::ID(130697), 0x2C8, Modified };
	};

	// don't create more than one map
	struct CreateMapHook {
		static inline std::string name{ "D728D640" };
		static FCData::GFxFontMap* Modified(FCData::GFxFontMap* map) {
			if (FCData::functionPass == 0) {
				auto result = Original(map);
				FCData::fontMap = FCUtility::MakeCopy(result);
				REX::INFO("{} GFxFontMap address = {:X} / {:X}", name, (std::uint64_t)result, (std::uint64_t)FCData::fontMap);
			}
			FCData::fontMap->AddRef();
			return FCData::fontMap;
		}
		inline static REL::THook Original{ REL::ID(130700), 0x54, Modified };
	};

	// don't create more than one lib
	struct CreateLibHook {
		static inline std::string name{ "872C9B7B" };
		static FCData::GFxFontLib* Modified(FCData::GFxFontLib* lib) {
			if (FCData::functionPass == 0) {
				auto result = Original(lib);
				FCData::fontLib = FCUtility::MakeCopy(result);
				REX::INFO("{} GFxFontLib address = {:X} / {:X}", name, (std::uint64_t)result, (std::uint64_t)FCData::fontLib);
			}
			FCData::fontLib->AddRef();
			return FCData::fontLib;
		}
		inline static REL::THook Original{ REL::ID(130700), 0x92, Modified };
	};

	// similar to sprintf but returns boolean
	struct FormatStringHook {
		static inline std::string name{ "0BC558A2" };
		static bool Modified(char* buffer, char* format, char* language) {
			if (FCData::functionPass == 0) {
				bool result = Original(buffer, format, language);
				REX::INFO("{} original config = {}", name, buffer);
				FCData::fontConfig = buffer;
				return result;
			}
			static auto newFormat = "SFSE/Plugins/" + std::string{ SFSE::GetPluginName() } + "/%s";
			bool result = Original(buffer, newFormat.data(), FCData::configs.back().data());
			REX::INFO("{} additional config = {}", name, buffer);
			return result;
		}
		inline static REL::THook Original{ REL::ID(130700), 0xCF, Modified };
	};

	struct MapFontHook {
		static inline std::string name{ "3153A5AB" };
		static bool Modified(FCData::GFxFontMap* map, char* key, char* tag, FCData::FontType type, float scale, std::int32_t unused) {
			auto info = std::format("{} input = {} + {} + {}", name, FCUtility::Quoted(key), FCUtility::Quoted(tag), (std::uint32_t)type);
			bool result = Original(map, key, tag, type, scale, unused);
			REX::INFO("{} / Result = {:X}", info, result);
			return result;
		}
		inline static REL::THook Original{ REL::ID(130700), 0x13A1, Modified };
	};

	struct GetStringHook {
		static inline std::string name{ "02752D51" };
		static FCData::GString* Modified(FCData::FontMapData** mapHolder, FCData::GString* str) {
			auto result = Original(mapHolder, str);
			if (str && result) {
				auto data = str->GetData();
				REX::INFO("{} duplication of {} in address {:X}", name, FCUtility::Quoted(data ? data : ""), (std::uint64_t)result);
				result = nullptr;
			}
			return result;
		}
		inline static REL::THook Original{ REL::ID(167014), 0x3F, Modified };
	};

	// sResourceStartUpArchiveList:Archive
	struct StartupListHook {
		static inline std::string name{ "CA0ED5B9" };
		static inline const std::map<std::uint8_t, std::string> list = {
			{ 1, "load" }, { 2, "reset" }
		};
		template <std::uint8_t type>
		static void Modified(char* value, std::uintptr_t unknown) {
			auto [newValue, newSize] = FCProcess::AdjustArchives(value);
			REX::INFO("{} on game {} = {}", name, list.at(type), newSize);
			if constexpr (type == 1) return OnLoad(newValue, unknown);
			if constexpr (type == 2) return OnReset(newValue, unknown);
		}
		inline static REL::THook OnLoad{ REL::ID(99457), 0x3E, Modified<1> };
		inline static REL::THook OnReset{ REL::ID(99468), 0x1651, Modified<2> };
	};

}
