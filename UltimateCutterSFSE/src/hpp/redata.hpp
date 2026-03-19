#pragma once

namespace UCWeaponData {

	struct SharedData {
		void* virtualFunctionsTable;
		RE::TBO_InstanceData* tboData;
	};
	static_assert(sizeof(SharedData) == 0x10);

	struct AmmoData {
		SharedData thisData;
		RE::TESLevItem* ammoList;
		RE::TESAmmo* ammoType;
		RE::BGSProjectile* projectile;
		RE::BGSArtObject* shellCasing;
		std::uint32_t ammoCapacity;
		std::uint8_t projectilesCount;
		std::uint8_t impactSoundsCount;
		bool npcCanUseAmmo;
		std::byte pad[1];
	};
	static_assert(sizeof(AmmoData) == 0x38);

	struct VariableRangeSubData {
		float valueMin;
		float valueMax;
		float inputMin;
		float inputMax;
		float acceleration;
		float decceleration;
	};
	static_assert(sizeof(VariableRangeSubData) == 0x18);

	struct VariableRangeData {
		SharedData thisData;
		VariableRangeSubData aperture;
		VariableRangeSubData distance;
		bool useVariableRange;
		std::byte pad[7];
	};
	static_assert(sizeof(VariableRangeData) == 0x48);

}