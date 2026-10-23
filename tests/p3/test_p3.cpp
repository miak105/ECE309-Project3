// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives


TEST(reg_empty_and_missing) {
    ToolRegistry r;

    CHECK(r.size() == 0);
    CHECK(r.bucket_count() >= 1);
    CHECK(r.bucket_count() <= 64);
    CHECK(r.max_probe_length() == 0);
    CHECK(r.find("missing") == nullptr);
}

TEST(reg_null_rejected) {
    ToolRegistry r;

    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;

    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    REQUIRE(r.add(std::make_unique<NamedTool>("beta")));
    REQUIRE(r.add(std::make_unique<NamedTool>("gamma")));

    int alpha = 0;
    int beta = 0;
    int gamma = 0;

    r.for_each([&](const Tool& tool) {
        if (tool.name() == "alpha") ++alpha;
        else if (tool.name() == "beta") ++beta;
        else if (tool.name() == "gamma") ++gamma;
        else CHECK(false);
    });

    CHECK(alpha == 1);
    CHECK(beta == 1);
    CHECK(gamma == 1);
    CHECK(r.size() == 3);
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;

    // Insert 10,000 different tools
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);

        REQUIRE(r.add(std::make_unique<NamedTool>(name)));

        if (i == 0) {
            first = r.find(name);
        }

        CHECK(r.load_factor() <= 1.0);
    }

    CHECK(r.size() == 10000);

    // Make sure all tools can still be found
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        CHECK(r.find(name) != nullptr);
    }

    // Check pointer stability after resizing
    CHECK(r.find("tool_0") == first);

    // Check the longest chain
    CHECK(r.max_probe_length() <= 64);

    std::printf("Buckets: %zu\n", r.bucket_count());
    std::printf("Load factor: %.4f\n", r.load_factor());
    std::printf("Max probe length: %zu\n",
                r.max_probe_length());
}

TEST(reg_alternative_hash) {
    ToolRegistry r;

    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));
    }

    // Simulate the weak hash: s[0] + s.size()
    std::vector<int> counts(r.bucket_count(), 0);

    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);

        std::size_t weak_hash =
            static_cast<unsigned char>(name[0]) + name.size();

        std::size_t index = weak_hash % counts.size();
        ++counts[index];
    }

    int weak_max = 0;
    for (int count : counts) {
        if (count > weak_max) {
            weak_max = count;
        }
    }

    std::printf("FNV-1a longest chain: %zu\n",
                r.max_probe_length());
    std::printf("Weak hash longest chain: %d\n",
                weak_max);

    CHECK(weak_max > static_cast<int>(r.max_probe_length()));
}


TEST(ws_accepts_normalized_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");

    REQUIRE(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
}

TEST(ws_accepts_root_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve(".", &out, &reason));
    CHECK(out == ws.root());

    REQUIRE(ws.resolve("a/..", &out, &reason));
    CHECK(out == ws.root());
}

TEST(ws_accepts_new_files) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    REQUIRE(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");

    REQUIRE(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_rejects_empty_path) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    CHECK(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_null_character) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    std::string bad = std::string("notes") + '\0' + "/x";

    CHECK(!ws.resolve(bad, &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_unsafe_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;

    std::vector<std::string> bad_paths = {
        "/etc/passwd",
        "..",
        "a/../../b",
        "notes/../../ws/notes/todo.txt"
    };

    for (const auto& path : bad_paths) {
        reason.clear();

        CHECK(!ws.resolve(path, &out, &reason));
        CHECK(!reason.empty());
    }
}


TEST(calc_left_to_right) {
    double v = 0;
    std::string err;

    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);

    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;

    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);

    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negative_numbers) {
    double v = 0;
    std::string err;

    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
}

TEST(calc_decimal_numbers) {
    double v = 0;
    std::string err;

    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
}

TEST(calc_grouped_division_by_zero) {
    double v = 0;
    std::string err;

    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_extra_expressions) {
    double v = 0;
    std::string err;

    REQUIRE(eval("8 * 3 - 5 / 5", &v, &err));
    CHECK(v == 23);

    REQUIRE(eval("18 / (2 + 1) + 7", &v, &err));
    CHECK(v == 13);
}

TEST(calc_long_expression) {
    std::string expr = "1";

    for (int i = 0; i < 20000; ++i) {
        expr += "+1";
    }

    double v = 0;
    std::string err;

    REQUIRE(eval(expr, &v, &err));
    CHECK(v == 20001);
}

TEST(calc_deep_parentheses) {
    std::string expr =
        std::string(5000, '(') +
        "7" +
        std::string(5000, ')');

    double v = 0;
    std::string err;

    REQUIRE(eval(expr, &v, &err));
    CHECK(v == 7);
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
