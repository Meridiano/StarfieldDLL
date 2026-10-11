#pragma once

#include "hpp/config.hpp"

namespace FCData {

	struct GStringData {
	private:
		std::uint64_t capacity;
		std::uint32_t refCount;
	public:
		char data[4];
		void AddRef() { std::atomic_ref(refCount) += 1; }
	};
	static_assert(sizeof(GStringData) == 0x10);

	struct GString {
	private:
		union {
			GStringData*   data;
			std::uintptr_t value;
		};
	public:
		char* GetData() {
			GString copy = *this; copy.value &= ~3;
			return copy.data ? copy.data->data : nullptr;
		}
	};
	static_assert(sizeof(GString) == 0x8);

	ScopedEnum(FontType, std::uint32_t,
		kNormal = 0,
		kItalic,
		kBold,
		kBoldItalic,
		kUnknown = 16
	);

	struct FontMapEntry {
		std::int64_t  flags;
		std::uint64_t index;
		GString       key;
		GString       tag;
		float         scale;
		FontType      type;
		std::int32_t  unusedA;
		std::int32_t  unusedB;
	};
	static_assert(sizeof(FontMapEntry) == 0x30);

	struct FontMapData {
		std::uint64_t count;
		std::uint64_t maxIndex;
		FontMapEntry  firstEntry;
	};
	static_assert(sizeof(FontMapData) == 0x40);

	struct FontLibData {
		void*         entries;
		std::uint64_t count;
		std::uint64_t capacity;
	};
	static_assert(sizeof(FontLibData) == 0x18);

	ScopedEnum(StateType, std::uint32_t,
		kNone = 0,
		kFontLib = 20,
		kFontMap = 23
	);

	class GFxState {
	private:
		void**        virtualTable;
		std::uint32_t refCount;
		std::byte     padN1[4];
		StateType     stateType;
		std::byte     padN2[4];
	public:
		void AddRef() { std::atomic_ref(refCount) += 1; }
		StateType GetType() { return stateType; }
	};
	static_assert(sizeof(GFxState) == 0x18);

	class GFxFontMap : public GFxState {
	public:
		FontMapData** mapHolder;
	};
	static_assert(sizeof(GFxFontMap) == 0x20);

	class GFxFontLib : public GFxState {
	public:
		FontLibData* libData;
	};
	static_assert(sizeof(GFxFontLib) == 0x20);

	class BSScaleformManager : public RE::BSScaleformManager {
	private:
		void* gfxLoader;
		void* imageLoader;
		void* translator;
	public:
		RE::BSString validNameChars;
	};
	static_assert(sizeof(BSScaleformManager) == 0x38);

	std::string fontConfig{};
	std::uint32_t functionPass{};
	std::vector<std::string> configs{};
	std::string validNameChars{};
	GFxFontMap* fontMap{};
	GFxFontLib* fontLib{};

}
