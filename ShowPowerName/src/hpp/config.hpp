#pragma once
#include "hpp/utility.hpp"

namespace SPN_Config {

	// regular
	RE::NiColor color = { 1.0F, 1.0F, 1.0F };
	float alpha = 1.0F;
	float fontSize = 23.0F;
	std::int32_t anchor = 6;
	ImVec2 position = { 1565.0F, 921.0F };
	float rotation = 3.3F;
	float pitch = 0.0F;
	float yaw = 0.0F;
	float distance = 1500.0F;
	// special
	std::int32_t keyCode = ImGuiKey_None;
	// unmanaged
	ImFont* targetFont = nullptr;

	namespace {

		toml::table GetRoot() {
			toml::table result;
			try { result = toml::parse_file(tomlPath); }
			catch (...) { REX::INFO("Config file error"); }
			return result;
		}

		toml::table* GetSection(toml::table& root, std::string_view name) {
			auto section = root[name].as_table();
			if (section) return section;
			toml::table newSection;
			auto [iterator, inserted] = root.insert_or_assign(name, newSection);
			return iterator->second.as_table();
		}

		void InsertValue(toml::table* section, std::string_view name, std::string_view key, auto value) {
			section->insert_or_assign(key, value);
			REX::INFO("Option [{}]{} = {}", name, key, value);
		}

		void AssignValue(toml::table* section, std::string_view name, std::string_view key, auto& value) {
			value = section->operator[](key).value_or(value);
			REX::INFO("Option [{}]{} = {}", name, key, value);
		}

	}

	void Save(bool special) {
		REX::INFO("Save config to file / {:X}", special);
		auto root = GetRoot();
		static std::string general = "General";
		if (auto section = GetSection(root, general); section) {
			InsertValue(section, general, "fColorRed", color.red);
			InsertValue(section, general, "fColorGreen", color.green);
			InsertValue(section, general, "fColorBlue", color.blue);
			InsertValue(section, general, "fAlpha", alpha);
			InsertValue(section, general, "fFontSize", fontSize);
			InsertValue(section, general, "iAnchor", anchor);
			InsertValue(section, general, "fPositionX", position.x);
			InsertValue(section, general, "fPositionY", position.y);
			InsertValue(section, general, "fRotation", rotation);
			InsertValue(section, general, "fPitch", pitch);
			InsertValue(section, general, "fYaw", yaw);
			InsertValue(section, general, "fDistance", distance);
		}
		if (special) {
			static std::string control = "Control";
			if (auto section = GetSection(root, control); section) {
				InsertValue(section, control, "iKeyCode", (std::uint32_t)keyCode);
			}
		}
		static auto fmtFlags = toml::toml_formatter::default_flags | toml::format_flags::relaxed_float_precision;
		static auto iosFlags = std::ios::out | std::ios::trunc;
		std::ofstream file(tomlPath, iosFlags);
		if (file) file << toml::toml_formatter(root, fmtFlags);
	}

	void Load(bool special) {
		REX::INFO("Load config from file / {:X}", special);
		auto root = GetRoot();
		static std::string general = "General";
		if (auto section = GetSection(root, general); section) {
			AssignValue(section, general, "fColorRed", color.red);
			AssignValue(section, general, "fColorGreen", color.green);
			AssignValue(section, general, "fColorBlue", color.blue);
			AssignValue(section, general, "fAlpha", alpha);
			AssignValue(section, general, "fFontSize", fontSize);
			AssignValue(section, general, "iAnchor", anchor);
			AssignValue(section, general, "fPositionX", position.x);
			AssignValue(section, general, "fPositionY", position.y);
			AssignValue(section, general, "fRotation", rotation);
			AssignValue(section, general, "fPitch", pitch);
			AssignValue(section, general, "fYaw", yaw);
			AssignValue(section, general, "fDistance", distance);
		}
		if (special) {
			static std::string control = "Control";
			if (auto section = GetSection(root, control); section) {
				AssignValue(section, control, "iKeyCode", keyCode);
			}
		}
	}

	void Reset(bool special) {
		REX::INFO("Reset config to defaults / {:X}", special);
		// regular
		color = { 1.0F, 1.0F, 1.0F };
		alpha = 1.0F;
		fontSize = 23.0F;
		anchor = 6;
		position = { 1565.0F, 921.0F };
		rotation = 3.3F;
		pitch = 0.0F;
		yaw = 0.0F;
		distance = 1500.0F;
		if (special) {
			keyCode = ImGuiKey_None;
		}
	}

}
