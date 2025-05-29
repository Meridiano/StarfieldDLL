#pragma once

static bool saveLoaded = false;

namespace CCRUtility {

    void ConsoleExecute(std::string command) {
        std::thread([](std::string commandLambda) {
            static REL::Relocation<void*> UnknownManager{ REL::ID(949606) };
            static REL::Relocation<void(*)(void*, const char*)> ExecuteCommand{ REL::ID(113576) };
            ExecuteCommand(UnknownManager.get(), commandLambda.data());
        }, command).detach();
    }

    RE::TESObjectCELL* GetParentCell(RE::TESObjectREFR* arg) {
        using func_t = RE::TESObjectCELL*(*)(std::int64_t, std::int64_t, RE::TESObjectREFR*);
        REL::Relocation<func_t> func{ REL::ID(118505) };
        return func(NULL, NULL, arg);
    }

    std::uint32_t PlayerCellFormID() {
        if (auto pc = RE::PlayerCharacter::GetSingleton(); pc)
            if (auto cell = GetParentCell(pc); cell)
                return (cell->formID);
        return 0;
    }

    std::string PlayerCellEditorID() {
        if (auto pc = RE::PlayerCharacter::GetSingleton(); pc)
            if (auto cell = GetParentCell(pc); cell)
                return cell->GetFormEditorID();
        return "";
    }

    auto TomlArrayToVector(toml::array* arr) {
        StrVec out;
        if (arr) for (toml::node& node : *arr) {
            std::string str = node.as_string()->get();
            out.push_back(str);
        }
        return out;
    }

    int GameLoadType() {
        if (saveLoaded) return 2;
        if (PlayerCellFormID() > 0) {
            auto gameDaysPassed = RE::TESForm::LookupByID<RE::TESGlobal>(0x39);
            if (gameDaysPassed && gameDaysPassed->GetValue() < 0.334F) return 1;
        }
        return 0;
    }

    template <typename T>
    T* GetMember(const void* base, std::ptrdiff_t offset) {
        auto address = std::uintptr_t(base) + offset;
        auto reloc = REL::Relocation<T*>(address);
        return reloc.get();
    };

    RE::BSTEventSource<RE::MenuOpenCloseEvent>* GetMenuEventSource(RE::UI* ui) {
        using type = RE::BSTEventSource<RE::MenuOpenCloseEvent>;
        return GetMember<type>(ui, 0x20);
    }

    RE::BSTEventSource<RE::TESEquipEvent>* GetEquipEventSource() {
        using type = decltype(&GetEquipEventSource);
        return REL::Relocation<type>{ REL::ID(64135) }();
    }

    RE::TESFile* LookupPlugin(std::uint8_t type, std::uint32_t id) {
        if (static auto tesDH = RE::TESDataHandler::GetSingleton(); tesDH) {
            using list = RE::BSTArray<RE::TESFile*>;
            switch (type) {
                case 0: {
                    auto full = GetMember<list>(tesDH, 0x1550);
                    if (full->size() > id) return full->operator[](id);
                }   break;
                case 1: {
                    auto small = GetMember<list>(tesDH, 0x1560);
                    if (small->size() > id) return small->operator[](id);
                }   break;
                case 2: {
                    auto medium = GetMember<list>(tesDH, 0x1570);
                    if (medium->size() > id) return medium->operator[](id);
                }   break;
            }
        }
        return nullptr;
    }

    std::string FormToString(RE::TESForm* form) {
        if (form) {
            RE::TESFile* plugin = nullptr;
            auto id = form->formID;
            switch (id >> 24) {
                case 0xFF:
                    break;
                case 0xFE:
                    plugin = LookupPlugin(1, (id >> 12) & 0xFFF);
                    id = id & 0xFFF;
                    break;
                case 0xFD:
                    plugin = LookupPlugin(2, (id >> 16) & 0xFF);
                    id = id & 0xFFFF;
                    break;
                default:
                    plugin = LookupPlugin(0, id >> 24);
                    id = id & 0xFFFFFF;
                    break;
            }
            if (plugin) {
                auto StringToLower = [](std::string str) {
                    std::string out = str;
                    std::transform(
                        str.begin(), str.end(), out.begin(),
                        [](unsigned char uch) { return std::tolower(uch); }
                    );
                    return out;
                };
                return std::format("{}:0x{:X}", StringToLower(plugin->fileName), id);
            }
        }
        return "";
    }

    auto GetAllStrings(RE::TESForm* form) {
        StrVec result;
        if (form) {
            auto AddNonEmpty = [](StrVec& v, std::string s) {
                if (s.size() > 0) v.push_back(s);
            };
            auto type = form->formType.get();
            switch (type) {

                #define ADD_STR(S) AddNonEmpty(result, S)
                #define ADD_FORM(F) ADD_STR(FormToString(F))
                #define ADD_EDID(F) ADD_STR(F->GetFormEditorID())
                #define ADD_STR_FORM(S,F) { ADD_STR(S); ADD_FORM(F); }
                #define ADD_EDID_FORM(F) { ADD_EDID(F); ADD_FORM(F); }
                #define CAST_ADD_KEYS(F,T) { auto obj = F->As<RE::##T>(); ADD_EDID(obj); for (auto key : obj->keywords) ADD_STR(key ? key->formEditorID.data() : ""); }

                case RE::FormType::kACHR: {
                    if (form->formID == 0x14) {
                        auto base = RE::TESForm::LookupByID(0x7);
                        ADD_STR_FORM("PlayerRef", form);
                        ADD_STR_FORM("Player", base);
                    } else {
                        auto actor = form->As<RE::Actor>();
                        ADD_EDID_FORM(actor);
                        if (auto bound = actor->GetBaseObject().get(); bound) {
                            auto npc = bound->As<RE::TESNPC>();
                            ADD_EDID_FORM(npc);
                            if (auto face = npc->faceNPC; face) {
                                ADD_EDID_FORM(face);
                            }
                        }
                    }
                }   break;
                case RE::FormType::kWEAP:
                    CAST_ADD_KEYS(form, TESObjectWEAP);
                    break;
                case RE::FormType::kARMO:
                    CAST_ADD_KEYS(form, TESObjectARMO);
                    break;
                case RE::FormType::kAMMO:
                    CAST_ADD_KEYS(form, TESAmmo);
                    break;
                case RE::FormType::kALCH:
                    CAST_ADD_KEYS(form, AlchemyItem);
                    break;
                case RE::FormType::kBOOK:
                    CAST_ADD_KEYS(form, TESObjectBOOK);
                    break;
                case RE::FormType::kSPEL:
                    CAST_ADD_KEYS(form, SpellItem);
                    break;

                #undef ADD_STR
                #undef ADD_FORM
                #undef ADD_EDID
                #undef ADD_STR_FORM
                #undef ADD_EDID_FORM
                #undef CAST_ADD_KEYS
            }
        }
        return result;
    }

    std::string VectorToString(StrVec vec, std::string del = ";") {
        std::string str;
        auto len = vec.size();
        if (len > 0) {
            str = vec[0];
            for (std::size_t ind = 1; ind < len; ind++) str += std::format("{}{}", del, vec[ind]);
        }
        return std::format("[{}]", str);
    }

    class VectorSearch {
    private:
        StrVec vec;
        StrVec::const_iterator vbeg;
        StrVec::const_iterator vend;
    public:
        VectorSearch(StrVec arg) {
            vec = arg;
            vbeg = vec.begin();
            vend = vec.end();
        }
        bool Contains(std::string arg) {
            return (std::find(vbeg, vend, arg) != vend);
        }
    };

    template <typename T1, typename T2, typename T3>
    class Triplet {
    public:
        T1 pos; T2 neg; T3 unk;
        Triplet(T1 a1, T2 a2, T3 a3) {
            pos = a1; neg = a2; unk = a3;
        }
    };

}