# vdf-parser

Parses VDF files using PEGTL

## Usage

```C++
#include "vdf_parser.hpp"

// Parse a VDF file.
auto result = vdf::parse_file("some-file.vdf");

if (result.has_value()) {
    // Use result.value()
    auto& root = result.value()[0]; 
    root["parent"]["child"]["something"].value
} else {
    // Let the people know.
    throw std::runtime_error{result.error()};
}
```
