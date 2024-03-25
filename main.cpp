#include "parser.hpp"
#include "diagnostics.hpp"

int main(int argc, const char* argv[]) {
    YAOPT::forceUTF8();
    const int argi = 2;
    if (argc < argi) {
        YAOPT::Error error;
        error.with(YAOPT::ErrorMessage().fatal().text("too few arguments, input file expected"));
        error.with(YAOPT::ErrorMessage().usage().text("YAOPT <input>"));
        error.report(nullptr, true);
        std::exit(10);
    }
    const char* input_file = argv[1];
    YAOPT::Parser parser(YAOPT::readText(input_file));
    try {
        parser.tokenize();
        parser.parse();
    } catch (YAOPT::Error& error) {
        error.report(&parser.source, true);
        std::exit(20);
    }
    std::string buf;
    for (auto&& entity : parser.entities) {
        buf += entity->serialize();
    }
    FILE* out = YAOPT::open("out.md", "w");
    fprintf(out, "# CFG of %s\n", input_file);
    fprintf(out, "%s", buf.data());
}
