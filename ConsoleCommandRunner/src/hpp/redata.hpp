#pragma once

namespace CCRExtra {

	class ExtraLeveledCreature : public RE::BSExtraData {
	public:
		ScopedEnum(TemplateType, std::uint8_t,
			kTraits = 0,
			kStats,
			kFactions,
			kSpells,
			kAI_Data,
			kAI_Packages,
			kSecondInventory,
			kBaseData,
			kInventory,
			kScript,
			kAI_DefaultPackageList,
			kAttackData,
			kKeywords,
			kReactionRadius,
			kCombatStyle,
			kTotal
		);
		static constexpr auto RTTI = RE::RTTI::ExtraLeveledCreature;
		static constexpr auto VTABLE = RE::VTABLE::ExtraLeveledCreature;
		static constexpr auto EXTRADATATYPE = RE::ExtraDataType::kLeveledCreature;
		// members
		RE::TESNPC* originalBase;
		RE::TESNPC* templates[TemplateType::kTotal];
	};
	static_assert(offsetof(ExtraLeveledCreature, originalBase) == 0x18);
	static_assert(offsetof(ExtraLeveledCreature, templates) == 0x20);
	static_assert(sizeof(ExtraLeveledCreature) == 0x98);

}