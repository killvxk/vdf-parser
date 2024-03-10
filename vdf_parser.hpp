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
    std::vector<Object> arr{};

    Object &operator[](const std::string &key) { return kvs[key]; }
    Object &operator[](std::int64_t index) { return arr[index]; }
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
struct ObjectMembers : list_tail<sor<KeyValue, Object>, Seps> {};
struct ObjectMembersStart : one<'{'> {};
struct ObjectMembersEnd : one<'}'> {};
struct ObjectMembersList : seq<ObjectMembersStart, Seps, until<ObjectMembersEnd, ObjectMembers>> {};

struct NamedObject : seq<ObjectName, Seps, ObjectMembersList> {};
struct UnnamedObject : seq<ObjectMembersList> {};
struct Object : sor<NamedObject, UnnamedObject> {};
struct Objects : list_tail<Object, Seps> {};

struct Grammar : seq<Seps, Objects, eof> {};

struct State {
    std::string item{};
    std::string key{};
    std::string value{};
    std::stack<vdf::Object> objs{};
    std::string obj_name{};

    State() {
        objs.emplace(); // Add a root object.
    }
};

template <typename Rule> struct Action : nothing<Rule> {};

template <> struct Action<Character> {
    template <typename Input> static void apply(const Input &in, State &s) {
        s.item += in.string_view();
    }
};

template <> struct Action<Key> {
    template <typename Input> static void apply(const Input &, State &s) {
        s.key = std::move(s.item);
    }
};

template <> struct Action<Value> {
    template <typename Input> static void apply(const Input &, State &s) {
        s.value = std::move(s.item);
    }
};

template <> struct Action<KeyValue> {
    template <typename Input> static void apply(const Input &, State &s) {
        vdf::Object obj{};
        obj.name = std::move(s.key);
        obj.value = std::move(s.value);

        s.objs.top().kvs[obj.name] = std::move(obj);
    }
};

template <> struct Action<ObjectMembersStart> {
    template <typename Input> static void apply(const Input &, State &s) {
        auto &obj = s.objs.emplace();
        obj.name = std::move(s.obj_name);
    }
};

template <> struct Action<ObjectName> {
    template <typename Input> static void apply(const Input &, State &s) {
        s.obj_name = std::move(s.item);
    }
};

template <> struct Action<Object> {
    template <typename Input> static void apply(const Input &, State &s) {
        auto obj = std::move(s.objs.top());
        s.objs.pop();

        if (obj.name.empty()) {
            s.objs.top().arr.emplace_back(std::move(obj));
        } else {
            s.objs.top().kvs[obj.name] = std::move(obj);
        }
    }
};
} // namespace parser

inline std::expected<Object, tao::pegtl::parse_error> parse_str(std::string_view str) {
    tao::pegtl::memory_input in{str, ""};
    parser::State state{};

    try {
        tao::pegtl::parse<parser::Grammar, parser::Action>(in, state);
        return state.objs.top();
    } catch (const tao::pegtl::parse_error &e) {
        return std::unexpected{e};
    }
}

inline std::expected<Object, tao::pegtl::parse_error> parse_file(std::string_view path) {
    tao::pegtl::file_input in{path};
    parser::State state{};

    try {
        tao::pegtl::parse<parser::Grammar, parser::Action>(in, state);
        return state.objs.top();
    } catch (const tao::pegtl::parse_error &e) {
        return std::unexpected{e};
    }
}
} // namespace vdf
