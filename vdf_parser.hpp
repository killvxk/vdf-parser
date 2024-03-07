#pragma once

#include <expected>
#include <stack>
#include <string>
#include <unordered_map>

#include <tao/pegtl.hpp>

// https://developer.valvesoftware.com/wiki/KeyValues
namespace vdf {
struct Object {
    std::string name{};
    std::string value{};
    std::unordered_map<std::string, Object> kvs{};

    Object &operator[](const std::string &key) { return kvs[key]; }
};

namespace parser {
using namespace tao::pegtl;

struct Comment : disable<two<'/'>, until<eolf>> {};
struct Sep : sor<space, Comment> {};
struct Seps : star<Sep> {};

struct Escaped : if_must<one<'\\'>, sor<one<'"', '\\'>>> {};
struct Regular : not_one<'\r', '\n'> {};
struct Character : sor<Escaped, Regular> {};
struct String : if_must<one<'"'>, until<one<'"'>, Character>> {};

struct Key : String {};
struct Value : String {};
struct KeyValue : seq<Key, Seps, Value> {};

struct Object;
struct ObjectName : String {};
struct ObjectMembers : sor<list<sor<KeyValue, Object>, Seps>, Seps> {};
struct Object : seq<opt<ObjectName>, Seps, one<'{'>, until<one<'}'>, ObjectMembers>> {};

struct Grammar : until<eof, sor<eolf, Sep, Object>> {};

struct State {
    std::string item{};
    std::string key{};
    std::string value{};
    std::stack<vdf::Object> objs{};
    std::vector<vdf::Object> root{};
};

template <typename Rule> struct Action : nothing<Rule> {};

template <> struct Action<Character> {
    template <typename Input> static void apply(const Input &in, State &s) {
        s.item += in.string_view();
    }
};

template <> struct Action<Key> {
    template <typename Input> static void apply(const Input &in, State &s) {
        s.key = std::move(s.item);
    }
};

template <> struct Action<Value> {
    template <typename Input> static void apply(const Input &in, State &s) {
        s.value = std::move(s.item);
    }
};

template <> struct Action<KeyValue> {
    template <typename Input> static void apply(const Input &in, State &s) {
        vdf::Object obj{};
        obj.name = std::move(s.key);
        obj.value = std::move(s.value);

        if (s.objs.empty()) {
            // The object has no name (happens w/ root objects sometimes). Just add an empty parent
            // to add this kv to.
            s.objs.emplace();
        }

        s.objs.top().kvs[obj.name] = std::move(obj);
    }
};

template <> struct Action<ObjectName> {
    template <typename Input> static void apply(const Input &in, State &s) {
        vdf::Object obj{};
        obj.name = std::move(s.item);
        s.objs.push(std::move(obj));
    }
};

template <> struct Action<Object> {
    template <typename Input> static void apply(const Input &in, State &s) {
        auto obj = std::move(s.objs.top());
        s.objs.pop();

        if (s.objs.empty()) {
            s.root.emplace_back(std::move(obj));
        } else {
            s.objs.top().kvs[obj.name] = std::move(obj);
        }
    }
};
} // namespace parser

inline std::expected<std::vector<Object>, std::string> parse_str(std::string_view str) {
    tao::pegtl::memory_input in{str, ""};
    parser::State state{};

    try {
        tao::pegtl::parse<parser::Grammar, parser::Action>(in, state);
        return state.root;
    } catch (const tao::pegtl::parse_error &e) {
        return std::unexpected{e.what()};
    }
}

inline std::expected<std::vector<Object>, std::string> parse_file(std::string_view path) {
    tao::pegtl::file_input in{path};
    parser::State state{};

    try {
        tao::pegtl::parse<parser::Grammar, parser::Action>(in, state);
        return state.root;
    } catch (const tao::pegtl::parse_error &e) {
        return std::unexpected{e.what()};
    }
}
} // namespace vdf
