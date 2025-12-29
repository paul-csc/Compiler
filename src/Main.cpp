#include "pch.h"
#include "Tokenizer.h"
#include "Parser.h"

int main() {
    const char* fileName = "src.gl";

    // open files
    std::filesystem::path path = std::filesystem::current_path() / fileName;
    std::ifstream srcFile(path, std::ios::in | std::ios::binary);
    if (!srcFile) {
        std::cerr << "Failed to open file: " << path << "\n";
        return 1;
    }

    std::string input((std::istreambuf_iterator<char>(srcFile)), std::istreambuf_iterator<char>());

    Compiler::Tokenizer tokenizer(std::move(input));
    const auto tokens = tokenizer.Tokenize();

    std::cout << "Tokens: \n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];

        std::cout << i + 1 << ": " << token.lexeme;
        if (token.type == Compiler::TokenType::LITERAL) {
            std::cout << " (" << token.value << ")";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    Compiler::Parser parser(tokens);
    const auto program = parser.ParseProgram();
    program->print(std::cout);

    std::cin.get();
    return 0;
}
