#include "generator.h"
#include "lexer.h"
#include "parser.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

int main() {
    std::filesystem::path inputFilePath = "test/main.c";
    std::filesystem::path outputFilePath = "test/main.asm";

    std::ifstream inputFile(inputFilePath, std::ios::in);
    if (!inputFile) {
        Compiler::Error("Failed to open file: " + inputFilePath.string());
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    sourceCode += '\n';
    inputFile.close();

    Compiler::Lexer lexer(sourceCode);
    const auto tokens = lexer.Lex();
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << std::format("{}: {}\n", i + 1, Compiler::TokenToStr(tokens[i].Type));
    }

    Compiler::Parser parser(tokens);
    Compiler::Generator generator(parser.ParseProgram());

    std::ofstream outputFile(outputFilePath);
    if (!outputFile) {
        Compiler::Error("Failed to write to file: " + outputFilePath.string());
    }
    outputFile << generator.GenerateAsm();
    outputFile.close();

    std::cout << "Output written to " << outputFilePath << "\n";

    std::cin.get();
    return 0;
}
