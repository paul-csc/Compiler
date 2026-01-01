#pragma once

#include "Parser.h"

namespace Glassy {

class Generator {
  public:
    Generator(std::unique_ptr<Program> program) : m_Program(std::move(program)) {}

    std::string GenerateAssembly() const {
        std::string result = "section .text\nglobal _start\n_start:\n";

        for (const auto& stmt : m_Program->statements) {
            stmt->GenerateAsm(result);
        }

        return result;
    }

  private:
    std::unique_ptr<Program> m_Program;
};

} // namespace Glassy
