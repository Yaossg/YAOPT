#include "lexer.hpp"
#include "source.hpp"

namespace YAOPT {

std::string_view Source::of(Token token) const noexcept {
    return lines.at(token.line).operator std::string_view().substr(token.column, token.width);
}

void Source::append(std::string const& code) {
    for (auto original : splitLines(code)) {
        std::string transformed;
        size_t width = 0;
        for (auto ch : original) {
            if (ch == '\t') {
                size_t padding = 4 - (width & 3);
                transformed += std::string(padding, ' ');
                width += padding;
            } else {
                transformed += ch;
                ++width;
            }
        }
        lines.emplace_back(std::move(transformed));
        LineTokenizer(*this, lines.back());
    }
}

bool Source::remains() {
    return !greedy.empty() || lines.back().ends_with('\\');
}

}