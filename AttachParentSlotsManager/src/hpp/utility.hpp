#pragma once

namespace SFUtility {

	class AttachParentArray;
	using VectorType = std::vector<RE::BGSKeyword*>;
	std::unordered_map<AttachParentArray*, VectorType> dataHolder;

	template <typename T>
	T* GetMember(const void* base, std::ptrdiff_t offset) {
		auto address = std::uintptr_t(base) + offset;
		REL::Relocation<T*> reloc{ address };
		return reloc.get();
	}

	class AttachParentArray {
	private:
		// definitions
		using SpanType = std::span<RE::BGSKeyword*>;
		enum class HasKey : std::uint8_t {
			kNull = 0,
			kYes,
			kNo
		};
		// members
		std::uintptr_t virtualTable;
		RE::BGSKeyword** begin;
		RE::BGSKeyword** end;
		RE::BGSKeyword** capacityEnd;
		// functions
		SpanType GetSpan() {
			return SpanType(begin, end);
		}
		template <typename T>
		T GetSizeFromSpan(SpanType list) {
			if constexpr (std::is_same<T, std::size_t>::value) return list.size();
			if constexpr (std::is_same<T, std::int32_t>::value) return T(list.size() & 0xFFFFFF);
			throw std::exception("unknown template type");
		}
		HasKey HasKeywordFromSpan(SpanType list, RE::BGSKeyword* kwda) {
			if (!kwda) return HasKey::kNull;
			for (auto item : list) if (item == kwda) return HasKey::kYes;
			return HasKey::kNo;
		}
		void ApplyChanges(VectorType list) {
			dataHolder.insert_or_assign(this, list);
			auto& newList = dataHolder.at(this);
			begin = newList.data();
			end = begin + newList.size();
			capacityEnd = end;
		}
	public:
		bool HasKeyword(RE::BGSKeyword* kwda) {
			auto list = GetSpan();
			return HasKeywordFromSpan(list, kwda) == HasKey::kYes;
		}
		template <typename T>
		T GetKeywordsCount() {
			auto list = GetSpan();
			return GetSizeFromSpan<T>(list);
		}
		template <typename T>
		RE::BGSKeyword* GetNthKeyword(T idx) {
			auto list = GetSpan();
			T size = GetSizeFromSpan<T>(list);
			return size > idx ? list[idx] : nullptr;
		}
		VectorType GetAllKeywords() {
			auto list = GetSpan();
			VectorType workspace;
			for (auto item : list) workspace.push_back(item);
			return workspace;
		}
		bool RemoveAllKeywords() {
			auto list = GetSpan();
			if (list.size() == 0) return false;
			VectorType workspace;
			ApplyChanges(workspace);
			return true;
		}
		bool RemoveKeyword(RE::BGSKeyword* kwda) {
			auto list = GetSpan();
			if (HasKeywordFromSpan(list, kwda) == HasKey::kYes) {
				VectorType workspace;
				for (auto item : list) if (item != kwda) workspace.push_back(item);
				ApplyChanges(workspace);
				return true;
			}
			return false;
		}
		template <typename T>
		bool RemoveNthKeyword(T idx) {
			auto list = GetSpan();
			T size = GetSizeFromSpan<T>(list);
			if (idx < size) {
				VectorType workspace;
				for (T it = 0; it < size; it++) if (it != idx) workspace.push_back(list[it]);
				ApplyChanges(workspace);
				return true;
			}
			return false;
		}
		bool AddKeyword(RE::BGSKeyword* kwda) {
			auto list = GetSpan();
			if (HasKeywordFromSpan(list, kwda) == HasKey::kNo) {
				VectorType workspace;
				for (auto item : list) workspace.push_back(item);
				workspace.push_back(kwda);
				ApplyChanges(workspace);
				return true;
			}
			return false;
		}
		bool ReplaceKeyword(RE::BGSKeyword* rem, RE::BGSKeyword* add) {
			auto list = GetSpan();
			auto bRem = HasKeywordFromSpan(list, rem) == HasKey::kYes;
			auto bAdd = HasKeywordFromSpan(list, add) == HasKey::kNo;
			if (bRem && bAdd) {
				VectorType workspace;
				for (auto item : list) if (item != rem) workspace.push_back(item);
				workspace.push_back(add);
				ApplyChanges(workspace);
				return true;
			}
			return false;
		}
		void AssignKeywords(VectorType list) {
			VectorType workspace;
			for (auto item : list) if (item) workspace.push_back(item);
			ApplyChanges(workspace);
		}
		void Reset() {
			begin = end = capacityEnd = nullptr;
			dataHolder.erase(this);
		}
	};

	auto FormListToArray(RE::BGSListForm* list, RE::FormType match = RE::FormType::kNONE) {
		std::vector<RE::TESForm*> result;
		auto& source = list->arrayOfForms;
		if (match == RE::FormType::kNONE) {
			// use all valid
			for (auto item : source) if (item) result.push_back(item);
		} else {
			// use only matching
			for (auto item : source) if (item && item->formType == match) result.push_back(item);
		}
		return result;
	}

}