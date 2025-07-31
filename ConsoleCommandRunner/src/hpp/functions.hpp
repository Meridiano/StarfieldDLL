#pragma once

#include "hpp/utility.hpp"

static StrVec dataLoadedMap;
static std::map<int, StrVec> gameLoadedMap;
static std::map<std::pair<std::string, int>, StrVec> menuActionMap;
static std::map<std::pair<std::string, StrVec>, CCRUtility::Triplet<StrVec, StrVec, StrVec>> equipItemMap;
static std::map<CCRUtility::Triplet<bool, int, int>, std::pair<bool, StrVec>> keyPressMap;

namespace CCRFunctions {

    void StoreCommands() {
        std::string path = "Data/SFSE/Plugins/ConsoleCommandRunner";
        if (fs::exists(path) && fs::is_directory(path)) {
            std::string type = ".toml";
            for (fs::directory_entry fileEntry : fs::directory_iterator(path)) {
                fs::path filePath = fileEntry.path();
                if (fileEntry.is_regular_file() && filePath.extension() == type) {
                    auto fileName = filePath.filename().string();
                    REX::INFO("Reading config file >> {}", fileName);
                    toml::parse_result data;
                    try {
                        data = toml::parse_file(filePath.string());
                    } catch (...) {
                        REX::INFO("Parsing error, file name >> {}", fileName);
                        continue;
                    }
                    toml::array* events = data.get_as<toml::array>("Event");
                    if (events) for (toml::node& event : *events) {
                        toml::table eventTable = *event.as_table();
                        std::string eventType = eventTable["EventType"].value_or<std::string>("");
                        if (eventType == "DataLoaded") {
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) if (com.is_string()) {
                                std::string command = com.as_string()->get();
                                REX::INFO("{} store >> {}", eventType, command);
                                dataLoadedMap.push_back(command);
                            }
                        } else if (eventType == "GameLoaded") {
                            auto loadTypeData = eventTable["aiLoadType"].as_integer();
                            int loadType = loadTypeData ? loadTypeData->get() : 0;
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) if (com.is_string()) {
                                std::string command = com.as_string()->get();
                                REX::INFO("{} with options [ {} ] store >> {}", eventType, loadType, command);
                                if (gameLoadedMap.contains(loadType)) gameLoadedMap.at(loadType).push_back(command);
                                else gameLoadedMap.insert_or_assign(loadType, std::vector(1, command));
                            }
                        } else if (eventType == "OnMenuOpenCloseEvent") {
                            auto menuNameData = eventTable["asMenuName"].as_string();
                            auto isOpeningData = eventTable["abOpening"].as_boolean();
                            std::string menuName = menuNameData ? menuNameData->get() : "";
                            int isOpening =  isOpeningData ? isOpeningData->get() : -1;
                            std::pair menuAction(menuName, isOpening);
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) if (com.is_string()) {
                                std::string command = com.as_string()->get();
                                REX::INFO("{} with options [ {} {} ] store >> {}", eventType, menuName, isOpening, command);
                                if (menuActionMap.contains(menuAction)) menuActionMap.at(menuAction).push_back(command);
                                else menuActionMap.insert_or_assign(menuAction, std::vector(1, command));
                            }
                        } else if (eventType == "OnItemEquipped") {
                            auto isEquippingData = eventTable["abEquipping"].as_boolean();
                            auto actorStringData = eventTable["asActorString"].as_string();
                            auto itemStringsData = eventTable["aItemStrings"].as_array();
                            int isEquipping = isEquippingData ? isEquippingData->get() : -1;
                            std::string actorString = actorStringData ? actorStringData->get() : "";
                            StrVec itemStrings = CCRUtility::TomlArrayToVector(itemStringsData);
                            std::pair equipElement(actorString, itemStrings);
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) if (com.is_string()) {
                                std::string command = com.as_string()->get();
                                REX::INFO("{} with options [ {} {} {} ] store >> {}", eventType, isEquipping, actorString, CCRUtility::VectorToString(itemStrings), command);
                                if (equipItemMap.contains(equipElement)) {
                                    auto& equip = equipItemMap.at(equipElement).first;
                                    auto& unequip = equipItemMap.at(equipElement).second;
                                    auto& undefined = equipItemMap.at(equipElement).third;
                                    switch (isEquipping) {
                                        case 1:
                                            equip.push_back(command);
                                            break;
                                        case 0:
                                            unequip.push_back(command);
                                            break;
                                        default:
                                            undefined.push_back(command);
                                            break;
                                    }
                                } else {
                                    StrVec equip;
                                    StrVec unequip;
                                    StrVec undefined;
                                    switch (isEquipping) {
                                        case 1:
                                            equip.push_back(command);
                                            break;
                                        case 0:
                                            unequip.push_back(command);
                                            break;
                                        default:
                                            undefined.push_back(command);
                                            break;
                                    }
                                    equipItemMap.insert_or_assign(equipElement, CCRUtility::Triplet(equip, unequip, undefined));
                                }
                            }
                        } else if (eventType == "OnKeyPress") {
                            auto isGamepadData = eventTable["abGamepad"].as_boolean();
                            auto primaryKeyData = eventTable["aiPrimaryKey"].as_integer();
                            auto secondaryKeyData = eventTable["aiSecondaryKey"].as_integer();
                            bool isGamepad = isGamepadData ? isGamepadData->get() : false;
                            int primaryKey = primaryKeyData ? primaryKeyData->get() : -1;
                            int secondaryKey = secondaryKeyData ? secondaryKeyData->get() : -1;
                            CCRUtility::Triplet keyAction(isGamepad, primaryKey, secondaryKey);
                            toml::array* comArray = eventTable["Commands"].as_array();
                            if (comArray) for (toml::node& com : *comArray) if (com.is_string()) {
                                std::string command = com.as_string()->get();
                                REX::INFO("{} with options [ {} {} {} ] store >> {}", eventType, isGamepad, primaryKey, secondaryKey, command);
                                if (keyPressMap.contains(keyAction)) keyPressMap.at(keyAction).second.push_back(command);
                                else keyPressMap.insert_or_assign(keyAction, std::pair(true, std::vector(1, command)));
                            }
                        }
                    }
                }
            }
        }
    }

    void RunDataCommands() {
        for (std::string command : dataLoadedMap) {
            if (command.empty()) continue;
            REX::INFO("{} execute >> {}", "DataLoaded", command);
            CCRUtility::ConsoleExecute(command);
        }
    }

    void RunGameCommands(int loadType) {
        if (gameLoadedMap.contains(loadType)) {
            auto commandArray = gameLoadedMap.at(loadType);
            for (std::string command : commandArray) {
                if (command.empty()) continue;
                REX::INFO("{} with options [ {} ] execute >> {}", "GameLoaded", loadType, command);
                CCRUtility::ConsoleExecute(command);
            }
        }
    }

    void RunMenuCommands(std::string sName, int iOpen) {
        for (auto mapEntry : menuActionMap) {
            auto menuAction = mapEntry.first;
            std::string menuName = menuAction.first;
            int isOpening = menuAction.second;
            bool matchName = (menuName == sName || menuName == "");
            bool matchOpening = (isOpening == iOpen || isOpening == -1);
            if (matchName && matchOpening) {
                auto commandArray = mapEntry.second;
                for (std::string command : commandArray) {
                    if (command.empty()) continue;
                    REX::INFO("{} with options [ {} {} ] execute >> {}", "OnMenuOpenCloseEvent", menuName, isOpening, command);
                    CCRUtility::ConsoleExecute(command);
                }
            }
        }
    }

    void RunEquipCommands(StrVec vActor, StrVec vItem, bool bEquipping) {
        for (auto mapEntry : equipItemMap) {
            auto storedStrings = mapEntry.first;
            auto actorString = storedStrings.first;
            auto itemStrings = storedStrings.second;
            bool matchActor = (CCRUtility::VectorSearch(vActor).Contains(actorString) || actorString == "");
            bool matchItem = itemStrings.empty() || [](StrVec a1, StrVec a2) {
                auto a2s = CCRUtility::VectorSearch(a2);
                for (auto a1s : a1) if (!a2s.Contains(a1s)) return false;
                return true;
            } (itemStrings, vItem);
            if (matchActor && matchItem) {
                auto commandTriplet = mapEntry.second;
                auto ExecVector = [](StrVec commandArray, StrVec info) {
                    for (std::string command : commandArray) {
                        if (command.empty()) continue;
                        REX::INFO("{} with options [ {} {} {} ] execute >> {}", "OnItemEquipped", info[0], info[1], info[2], command);
                        CCRUtility::ConsoleExecute(command);
                    }
                };
                auto itemStringsMerged = CCRUtility::VectorToString(itemStrings);
                ExecVector(commandTriplet.third, { "-1", actorString, itemStringsMerged });
                if (bEquipping) ExecVector(commandTriplet.first, { "1", actorString, itemStringsMerged });
                else ExecVector(commandTriplet.second, { "0", actorString, itemStringsMerged });
            }
        }
    }

    void RunKeyPressCommands() {
        bool run = (keyPressMap.size() > 0);
        while (run) {
            if (CCRUtility::GameIsFocused()) for (auto& mapEntry : keyPressMap) {
                auto& keyAction = mapEntry.first;
                auto& isGamepad = keyAction.first;
                auto& primaryKey = keyAction.second;
                auto& secondaryKey = keyAction.third;
                bool& keyListener = mapEntry.second.first;
                bool keyPressed = CCRUtility::IsKeyPressed(isGamepad, false, primaryKey);
                if (keyPressed) {
                    if (keyListener) {
                        keyListener = false;
                        bool execute = CCRUtility::IsKeyPressed(isGamepad, true, secondaryKey);
                        if (execute) {
                            auto commandArray = mapEntry.second.second;
                            for (std::string command : commandArray) {
                                if (command.empty()) continue;
                                REX::INFO("{} with options [ {} {} {} ] execute >> {}", "OnKeyPress", isGamepad, primaryKey, secondaryKey, command);
                                CCRUtility::ConsoleExecute(command);
                            }
                        }
                    }
                } else keyListener = true;
            }
            Sleep(50);
        }
        REX::INFO("Keys map is empty, close listener thread");
    }

}