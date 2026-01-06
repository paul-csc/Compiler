#include "pch.h"
#include "Generator.h"
#include "Parser.h"
#include "Tokenizer.h"

int main(int argc, char** argv) {
    // if (argc != 2) {
    //     Compiler::Error("Usage: <program> <source_file>.txt");
    // }

    std::filesystem::path inputFilePath /*= argv[1]*/;

    inputFilePath = "test"; // for debug

    if (inputFilePath.has_extension()) {
        if (inputFilePath.extension() != ".txt") {
            Compiler::Error(inputFilePath.string() + " is not a Compiler source file");
        }
    } else {
        inputFilePath.replace_extension(".txt");
    }

    std::filesystem::path outputFilePath = inputFilePath;
    outputFilePath.replace_extension(".asm");

    std::ifstream inputFile(inputFilePath, std::ios::in | std::ios::binary);
    if (!inputFile) {
        Compiler::Error("Failed to open file: " + inputFilePath.string());
    }

    std::string sourceCode((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    sourceCode += '\n'; // ensure last line ends with newline
    inputFile.close();

    // tokenize, parse, and generate assembly
    Compiler::Parser parser(Compiler::Tokenizer::Tokenize(sourceCode));
    Compiler::Generator generator(parser.ParseProgram());

    // write output file
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
