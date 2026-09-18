//
// Created by Arthur on 20/12/2020.
//

#pragma once

#include <boost/fusion/adapted/struct.hpp>
//#include <boost/multiprecision/cpp_int.hpp>

//using namespace boost::multiprecision;

//using int128_t = __int128;

#include <iostream>
#include <string>

namespace structs {
struct Timestamp {
    double time_of_day;
    double ms;

    friend std::istream& operator>> (std::istream& is, Timestamp& time)
    {
        struct std::tm tm;
        if (is >> std::get_time(&tm, "%H:%M:%S") >> time.time_of_day)
            time.time_of_day += tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;

        return is;
    }

    friend std::ostream& operator<< (std::ostream& os, const Timestamp& time) {
//        auto tm = *std::localtime(&time.date);
        return os << time.time_of_day;// << std::put_time(&tm, "%H:%M:%S") << "\n";
    }
};

struct Character {
    bool is_player = false;
    std::string name;
    uint64_t id;
//    uint64_t id2;

    friend std::ostream& operator<< (std::ostream& os, const Character& character) {
        return os << std::boolalpha << character.is_player << " " << character.name;
    }
};

struct Ability {
    std::string name;
    uint64_t id;

    friend std::ostream& operator<< (std::ostream& os, const Ability& ability) {
        return os << ability.name << " " << ability.id;
    }
};

struct Event {
//    REMOVE_EFFECT(836045448945478L), APPLY_EFFECT(836045448945477L), SPEND(836045448945473L), RESTORE(836045448945476L), EVENT(
//    836045448945472L), UNKNOWN(0L);
    std::string type;
    uint64_t type_id;

//    REMOVE_EFFECT(836045448945478L), APPLY_EFFECT(836045448945477L), SPEND(836045448945473L), RESTORE(836045448945476L), EVENT(
//    836045448945472L), UNKNOWN(0L);
    std::string name;
    uint64_t id;

    friend std::ostream& operator<< (std::ostream& os, const Event& event) {
        return os << event.type << " " << event.type_id << "" << event.name << " " << event.id;
    }
};

struct NameIdPair {
    std::string name;
    uint64_t id;

    friend std::ostream& operator<< (std::ostream& os, const NameIdPair& nip) {
        return os << nip.name << " " << nip.id;
    }
};

using Types = std::vector<NameIdPair>;


//Result?
struct Amount {
//    INTERNAL(836045448940876L), ENERGY(836045448940874L), KINETIC(836045448940873L), ELEMENTAL(836045448940875L), NONE(
//    0L), UNKNOWN(0L);
    // EffectType?
    enum Type
    {
        generic = 0,
        energy,
        kinetic,
        elemental,
        internal
    };

//    MISS(836045448945502L), GLANCE(836045448945509L), DODGE(836045448945505L), DEFLECT(836045448945508L), PARRY(
//    836045448945503L), IMMUNE(836045448945506L), RESIST(836045448945507L), NONE(0L), UNKNOWN(0L);

    // Mitigation?
    enum Modifier
    {
        none = 0,
        miss,
        parry,
        dodge,
        shield,
        immune,
        deflect,
        resist
    };

//private int value;
//private boolean critical;
//
//private EffectType effectType = EffectType.NONE;
//private long effectId;
//
//private MitigationType mitigationType = MitigationType.NONE;
//private long mitigationId;
//
//private boolean absorb;
//private int absorbValue;
//private long absorbId;

    int value;
    bool is_critical;
    Types types;
    std::string type;
    uint64_t id;
//    Type type;
    Modifier modifer;

    friend std::ostream& operator<<(std::ostream& os, const Amount& amount) {
        os << amount.value << " " << amount.is_critical << " ";
        for (auto& t : amount.types) {
            os << t.name << " " << t.id << " ";
        }
        return os;
    }
};

struct Record {
    Timestamp time;
    Character who;
    Character target;
    Ability ability;
    NameIdPair type;
    NameIdPair action;
    Amount amount;

    friend std::ostream& operator<<(std::ostream& os, const Record& record) {
//        using namespace date;
        os << record.time
           << " " << record.who
           << " " << record.target
           << " " << record.ability
           << " " << record.type
           << " " << record.action
           << " " << record.amount;
        return os;
    }
};

typedef std::vector<Record> Records;
}

BOOST_FUSION_ADAPT_STRUCT(structs::Character,
(bool, is_player)
(std::string, name)
(uint64_t, id)
//    (uint64_t, id2)
)

BOOST_FUSION_ADAPT_STRUCT(structs::Ability,
(std::string, name)
(uint64_t, id)
)

BOOST_FUSION_ADAPT_STRUCT(structs::NameIdPair,
(std::string, name)
(uint64_t, id)
)

BOOST_FUSION_ADAPT_STRUCT(structs::Amount,
(int, value)
(bool, is_critical)
(structs::Types, types)
//    (std::string, type)
//    (uint64_t, id)
)

//Must be in the global namespace
BOOST_FUSION_ADAPT_STRUCT(structs::Record,
(structs::Timestamp, time)
(structs::Character, who)
(structs::Character, target)
(structs::Ability, ability)
(structs::NameIdPair, type)
(structs::NameIdPair, action)
(structs::Amount, amount)
)
