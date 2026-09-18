//
// Created by Arthur on 20/12/2020.
//

#pragma once

#include <boost/iostreams/stream.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/repository/include/qi_seek.hpp>

#include <spdlog/spdlog.h>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace boost { namespace spirit { namespace traits {
template <typename It>
struct assign_to_attribute_from_iterators<std::string, It, void> {
    static inline void call(It f, It l, std::string& attr) {
        attr = std::string(&*f, std::distance(f,l));
    }
};

template <typename It>
struct assign_to_attribute_from_iterators<structs::Timestamp, It, void> {
    static inline void call(It f, It l, structs::Timestamp& time) {
        struct std::tm tm;
        boost::iostreams::stream<boost::iostreams::array_source> stream(f, l);
        stream >> time;
    }
};
} } }

namespace QiParsers {
template <typename It, typename Skipper = qi::iso8859_1::blank_type>
struct RecordParser : qi::grammar<It, structs::Record(), Skipper>
{
    RecordParser() : RecordParser::base_type(start) {
        using namespace qi;
        namespace repo = boost::spirit::repository;
        using boost::phoenix::push_back;
        using boost::phoenix::val;

        id = eps >> '{' >> ulong_long >> '}';

//        name = *(/*iso8859_1::*/char_("A-Za-z-") | blank);
        name = *~char_("{]");

        nip = eps >> name >> id;

        player = eps > '['
                >> matches['@']
                >> name
                >> ']';

        companion = eps > '['
                >> matches['@']
                >> omit[name]
                >> omit[':']
                >> name >> id
                >> ']';

        npc = eps > '['
                >> matches['@']
                >> name
                >> id >> ":" >> omit[ulong_long]
                >> ']';

        character = //eps > (player || companion);
                eps > '['
                >> matches['@']
                >> name >> -((id >> ":" >> omit[ulong_long]) || (name >> id))
                >> ']';

        ability = eps > '['
                >> -(name >> id)
                >> ']';

        type = as_string[*alpha] >> id;
        action = as_string[*(char_ - '{')] >> id;

        amount = eps > '('
//                               >> (int_ >> matches['*'])
//                               || (*(as_string[+(char_ - '{')] >> id))
//                               || omit[*(char_ - ')')]
//                >> -(int_ >> matches['*']
//                >> (*(as_string[+(char_ - '{')] >> id) | omit[*(char_ - ')')]))
                >> -(int_ >> matches['*']
                          >> (*(as_string[+(char_ - '{')] >> id)))
                >> omit[*(char_ - ')')]
                >> ')';

        threat = eps > '<'
                >> int_
                >> '>';

        line = eps >
               '[' >> /*raw[*~char_(']')]*/stream >> ']'
                   >> character
                   >> character
                   >> ability
                   >> '[' >> type >> ": " >> action >> ']'
                   >> amount
                   //                    >> (threat | eol);
                   //                    >> eol;
                   >> repo::qi::seek[eoi];

//        ignore = *~char_("\r\n");
        start = line;
//        start = (line[push_back(_val, _1)] | ignore) % eol;

        BOOST_SPIRIT_DEBUG_NODES((name)(id)(character)(ability)(line));

        on_error<fail>(
                start
                , std::cout
                        << val("Error! Expecting ")
                        << _4                               // what failed?
                        << val(" here: \"") << construct<std::string>(_3, _2)  << val("\"")
                        << std::endl
        );
    }

private:
//    qi::rule<It> ignore;
    qi::rule<It, structs::Record(), Skipper> line;
    qi::rule<It, std::uint64_t()> id;
    qi::rule<It, std::string()> name;
    qi::rule<It> nip;
    qi::rule<It, structs::Character()> player;
    qi::rule<It, structs::Character()> companion;
    qi::rule<It, structs::Character()> npc;
    qi::rule<It, structs::Character()> character;
    qi::rule<It, structs::Ability()> ability;
    qi::rule<It, structs::NameIdPair(), Skipper> type;
    qi::rule<It, structs::NameIdPair()> action;
    qi::symbols<char, int> amount_type;
    qi::symbols<char, int> amount_modifier;
    qi::rule<It, structs::Amount(), Skipper> amount; //// (1558* energy {836045448940874})
    qi::rule<It> threat;
//    qi::rule<It, structs::Records()> start;
    qi::rule<It, structs::Record(), Skipper> start;
};
}

template<typename Container, typename It>
Container parse_ignoring(It b, It e, int line_number) try {
    using namespace boost::spirit;
    namespace repo = boost::spirit::repository;

    static const QiParsers::RecordParser<It> parser;

    Container records;

    std::chrono::high_resolution_clock::time_point t0 = std::chrono::high_resolution_clock::now();
    bool result = qi::phrase_parse(b, e,
//                                   parser,
                                   parser,
//                                   *repo::seek[parser],
                                   qi::iso8859_1::blank,
                                   records);
//    bool result = parse(b, e, parser, records);
    if (!result || b != e)
        spdlog::error("Line: {} Something failed. Unparsed: {}", line_number, std::string(b, e));
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0);
//    std::cout << "Elapsed time: " << elapsed.count() << " ms\n";

    return records;
}
catch (std::exception& ex) {
    spdlog::error(ex.what());
    return {};
}
