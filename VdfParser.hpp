#pragma once

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

    Object& operator[](const std::string& key) { return kvs[key]; }
};

namespace parser {
using namespace tao::pegtl;

struct Comment : disable<two<'/'>, until<eolf>> {};
struct Sep : sor<one<' ', '\t', '\r', '\n'>, Comment> {};
struct Seps : star<Sep> {};

struct Escaped : if_must<one<'\\'>, sor<one<'"', '\\'>>> {};
struct Regular : not_one<'\r', '\n'> {};
struct Character : sor<Escaped, Regular> {};
struct String : if_must<one<'"'>, until<one<'"'>, Character>> {};

struct Item : String {};

struct Object;

struct Key : Item {};
struct Value : Item {};
struct KeyValue : sor<seq<Key, Seps, Value>, Object> {};

struct ObjectName : Item {};
struct Object : seq<ObjectName, Seps, one<'{'>, Seps, list<KeyValue, Seps>, Seps, one<'}'>> {};

struct Something : must<Seps, Object, Seps> {};
struct Grammar : until<eof, sor<eolf, Something>> {};

struct State {
    std::string item{};
    std::string key{};
    std::string value{};
    std::stack<vdf::Object> objs{};
    vdf::Object final_obj{};
};

template <typename Rule> struct Action : nothing<Rule> {};

template <> struct Action<Character> {
    template <typename Input> static void apply(const Input& in, State& s) { s.item += in.string_view(); }
};

template <> struct Action<Key> {
    template <typename Input> static void apply(const Input& in, State& s) { s.key = std::move(s.item); }
};

template <> struct Action<Value> {
    template <typename Input> static void apply(const Input& in, State& s) { s.value = std::move(s.item); }
};

template <> struct Action<KeyValue> {
    template <typename Input> static void apply(const Input& in, State& s) {
        vdf::Object obj{};
        obj.name = std::move(s.key);
        obj.value = std::move(s.value);
        s.objs.top().kvs[obj.name] = std::move(obj);
    }
};

template <> struct Action<ObjectName> {
    template <typename Input> static void apply(const Input& in, State& s) {
        vdf::Object obj{};
        obj.name = std::move(s.item);
        s.objs.emplace(std::move(obj));
    }
};

template <> struct Action<Object> {
    template <typename Input> static void apply(const Input& in, State& s) {
        auto obj = std::move(s.objs.top());
        s.objs.pop();

        if (s.objs.empty()) {
            s.final_obj[obj.name] = std::move(obj);
        } else {
            s.objs.top().kvs[obj.name] = std::move(obj);
        }
    }
};
} // namespace parser
} // namespace vdf
