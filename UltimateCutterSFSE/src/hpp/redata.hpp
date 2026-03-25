#pragma once

namespace UCWeaponData {

	struct SharedData {
		void* virtualFunctionsTable;
		RE::TBO_InstanceData* tboData;
	};
	static_assert(sizeof(SharedData) == 0x10);

	enum class ReticleType : std::uint32_t {
		kNone = 0,
		kUnarmed,
		kPistol,
		kShotgun,
		kRifle,
		kLaser,
		kTool,
		kCustom
	};

	struct AimData {
		SharedData thisData;
		float sightedTransitionSeconds;
		std::byte padN1[4];
		RE::BGSAimDownSightModel* aimDownSightTemplate;
		RE::AimDownSightData* aimDownSightData;
		RE::BGSAimModel* aimModelTemplate;
		RE::AimModelData* aimModelData;
		RE::BGSAimAssistModel* aimAssistModel;
		RE::BGSMeleeAimAssistModel* aimAssistModelMelee;
		RE::AimAssistData* aimAssistData;
		RE::MeleeAimAssistData* aimAssistDataMelee;
		std::int8_t accuracyBonus;
		bool hasScope;
		std::byte padN2[6];
		RE::TESForm* aimOpticalSightModel;
		bool updateAdvancedAimInfo;
		bool orientFireNodesToAimingDirection;
		bool enableMarkingTargets;
		bool inverseReticleSpread;
		bool uiReticleIgnoresPlayerMovement;
		std::byte padN3[3];
		ReticleType reticleType;
		std::byte padN4[4];
	};
	static_assert(sizeof(AimData) == 0x78);

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