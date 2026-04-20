#pragma once

namespace EZCData {

	struct ModInfoStruct {
		RE::BGSMod::Attachment::Mod* mod;
		RE::BSTArray<RE::BGSKeyword*> allowed;
		RE::BSTArray<RE::BGSKeyword*> disallowed;
		std::byte unknown[16];
	};
	static_assert(sizeof(ModInfoStruct) == 0x38);

	struct RankInfoStruct {
	public:
		RE::BSTArray<ModInfoStruct> mods;
	private:
		ComponentList* one;
		ComponentList* two;
	public:
		ComponentList* GetLegendaryRollsResources() { return one; }
		ComponentList* GetLegendaryPicksResources() { return two; }
		ComponentList* GetQualityUpgradeResources() { return one; }
	};
	static_assert(sizeof(RankInfoStruct) == 0x20);

	struct LegendaryCraftStruct {
		RankInfoStruct legendaryRanks[4];
		RE::BSTArray<RankInfoStruct> qualityRanks;
	};
	static_assert(sizeof(LegendaryCraftStruct) == 0x90);

}