#include "generator.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "symbol_table.h"
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
    Compiler::Parser parser(lexer.Lex());
    auto program = parser.ParseProgram();
    Compiler::ScopeStack scopes;
    Compiler::SemanticAnalyzer analyzer(program, scopes);
    analyzer.Analyze();
    Compiler::Generator generator(program, scopes);

    std::ofstream outputFile(outputFilePath);
    if (!outputFile) {
        Compiler::Error("Failed to write to file: " + outputFilePath.string());
    }
    outputFile << generator.GenerateAsm();
    outputFile.close();

    std::cout << "Output written to " << outputFilePath << "\n";
    return 0;
}
