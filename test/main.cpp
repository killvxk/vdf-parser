#include <string_view>

#include <boost/ut.hpp>

#include "vdf_parser.hpp"

int main() {
    using namespace boost::ut;

    "multiple unnamed roots"_test = [] {
        auto result = vdf::parse_str(R"(
            {
                "world_maxs" "1504 1504 800"
                "world_mins" "-992 -2816 -64"
                "skyname" "assault"
                "maxpropscreenwidth" "-1"
                "detailvbsp" "detail.vbsp"
                "detailmaterial" "detail/detailsprites"
                "classname" "worldspawn"
                "mapversion" "24"
                "hammerid" "1"
            }
            {
                "origin" "888 -1248 668"
                "targetname" "3"
                "angles" "0 0 0"
                "classname" "info_teleport_destination"
                "hammerid" "23166"
            }
        )");

        expect(result.has_value()) << [&] { return result.error(); } << fatal;
        expect(result.value().size() == 2_u);

        auto &a = result.value()[0];

        expect(a.name.empty());
        expect(a.kvs.size() == 9_u);
        expect(a.kvs["world_maxs"].name == "world_maxs");
        expect(a.kvs["world_maxs"].value == "1504 1504 800");
        expect(a.kvs["world_mins"].name == "world_mins");
        expect(a.kvs["world_mins"].value == "-992 -2816 -64");
        expect(a.kvs["skyname"].name == "skyname");
        expect(a.kvs["skyname"].value == "assault");
        expect(a.kvs["maxpropscreenwidth"].name == "maxpropscreenwidth");
        expect(a.kvs["maxpropscreenwidth"].value == "-1");
        expect(a.kvs["detailvbsp"].name == "detailvbsp");
        expect(a.kvs["detailvbsp"].value == "detail.vbsp");
        expect(a.kvs["detailmaterial"].name == "detailmaterial");
        expect(a.kvs["detailmaterial"].value == "detail/detailsprites");
        expect(a.kvs["classname"].name == "classname");
        expect(a.kvs["classname"].value == "worldspawn");
        expect(a.kvs["mapversion"].name == "mapversion");
        expect(a.kvs["mapversion"].value == "24");
        expect(a.kvs["hammerid"].name == "hammerid");
        expect(a.kvs["hammerid"].value == "1");

        auto &b = result.value()[1];

        expect(b.name.empty());
        expect(b.kvs.size() == 5_u);
        expect(b.kvs["origin"].name == "origin");
        expect(b.kvs["origin"].value == "888 -1248 668");
        expect(b.kvs["targetname"].name == "targetname");
        expect(b.kvs["targetname"].value == "3");
        expect(b.kvs["angles"].name == "angles");
        expect(b.kvs["angles"].value == "0 0 0");
        expect(b.kvs["classname"].name == "classname");
        expect(b.kvs["classname"].value == "info_teleport_destination");
        expect(b.kvs["hammerid"].name == "hammerid");
        expect(b.kvs["hammerid"].value == "23166");
    };

    "libraryfolders.vdf"_test = [] {
        auto result = vdf::parse_str(R"(
            "libraryfolders"
            {
            "0"
            {
                "path"		"C:\\Program Files (x86)\\Steam"
                "apps"
                {
                        "228980"		"395981975"
                }
            }
            "1"
            {
                "path"		"E:\\SteamLibrary"
                "apps"
                {
                        "211"		"2173258931"
                }
            }
            }
        )");

        expect(result.has_value()) << [&] { return result.error(); } << fatal;
        expect(result.value().size() == 1_u);

        auto &root = result.value()[0];

        expect(root.name == "libraryfolders");
        expect(root.kvs.size() == 2_u);
        expect(root.kvs["0"].name == "0");
        expect(root.kvs["0"].kvs.size() == 2_u);
        expect(root.kvs["0"].kvs["path"].name == "path");
        expect(root.kvs["0"].kvs["path"].value == R"(C:\\Program Files (x86)\\Steam)");
        expect(root.kvs["0"].kvs["apps"].name == "apps");
        expect(root.kvs["0"].kvs["apps"].kvs.size() == 1_u);
        expect(root.kvs["0"].kvs["apps"].kvs["228980"].name == "228980");
        expect(root.kvs["0"].kvs["apps"].kvs["228980"].value == "395981975");
        expect(root.kvs["1"].name == "1");
        expect(root.kvs["1"].kvs.size() == 2_u);
        expect(root.kvs["1"].kvs["path"].name == "path");
        expect(root.kvs["1"].kvs["path"].value == R"(E:\\SteamLibrary)");
        expect(root.kvs["1"].kvs["apps"].name == "apps");
        expect(root.kvs["1"].kvs["apps"].kvs.size() == 1_u);
        expect(root.kvs["1"].kvs["apps"].kvs["211"].name == "211");
        expect(root.kvs["1"].kvs["apps"].kvs["211"].value == "2173258931");
    };
}
