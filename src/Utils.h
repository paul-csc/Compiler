#pragma once

#include "pch.h"
#include "lexer.h"

namespace Compiler {

class ArenaAllocator {
  public:
    explicit ArenaAllocator(size_t chunkSize) : m_Size(chunkSize) {
        m_Buffer = new std::byte[m_Size];
        m_Offset = m_Buffer;
    }

    ~ArenaAllocator() { delete[] m_Buffer; }

    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;

    template <typename T, typename... Args>
    T* alloc(Args&&... args) {
        if (m_Offset + sizeof(T) > m_Buffer + m_Size) {
            throw std::bad_alloc();
        }

        std::byte* start = m_Offset;
        m_Offset += sizeof(T);
        return new (start) T(std::forward<Args>(args)...);
    }

  private:
    const size_t m_Size;

    std::byte* m_Buffer;
    std::byte* m_Offset;
};

inline void Error(SourceLocation loc, const std::string& msg) {
    std::cerr << std::format("{} [Ln {}, Col {}]\n", msg, loc.Line, loc.Column);  
    std::cin.get();
    std::exit(1);
}

inline void Error(const std::string& msg) {
    std::cerr << msg << "\n";
    std::cin.get();
    std::exit(1);
}

} // namespace Compiler
