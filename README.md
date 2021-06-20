# vdf-parser
Parses VDF files using PEGTL

## Usage
```
#include "VdfParser.hpp"

vdf::Object root{};

try {
    tao::pegtl::file_input in{, ""};
    vdf::parser::State s{};
    if (tao::pegtl::parse<vdf::parser::Grammar, vdf::parser::Action>(in, s)) {
      root = s.final_obj;
    }
} catch (const tao::pegtl::parse_error& e) {
    // Let the people know.
}

// Use root.
root["parent"]["child"]["something"].value
```
