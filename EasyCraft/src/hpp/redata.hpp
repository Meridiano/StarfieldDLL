#pragma once

namespace EZCData {

	struct ModInfoStruct {
		RE::BGSMod::Attachment::Mod* mod;
		RE::BSTArray<RE::BGSKeyword*> keysA;
		RE::BSTArray<RE::BGSKeyword*> keysB;
		std::byte unknown[16];
	};
	static_assert(sizeof(ModInfoStruct) == 0x38);

	struct RankInfoStruct {
		RE::BSTArray<ModInfoStruct> mods;
		ComponentList* rolls;
		ComponentList* picks;
	};
	static_assert(sizeof(RankInfoStruct) == 0x20);

	struct LegendaryCraftStruct {
		RankInfoStruct ranks[4];
	};
	static_assert(sizeof(LegendaryCraftStruct) == 0x80);

}