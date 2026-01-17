#include "utils.h"
#include "lexer.h"
#include <format>
#include <iostream>

namespace Compiler {
    
ArenaAllocator::ArenaAllocator(size_t chunkSize) : m_Size(chunkSize) {
    m_Buffer = new std::byte[m_Size];
    m_Offset = m_Buffer;
}

ArenaAllocator::~ArenaAllocator() {
    delete[] m_Buffer;
}

[[noreturn]] void Error(SourceLocation loc, const std::string& msg) {
    std::cerr << std::format("{} [Ln {}, Col {}]\n", msg, loc.Line, loc.Column);
    std::cin.get();
    std::exit(1);
}

[[noreturn]] void Error(const std::string& msg) {
    std::cerr << msg << "\n";
    std::cin.get();
    std::exit(1);
}

} // namespace Compiler
