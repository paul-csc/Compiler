#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Compiler {

enum IdentifierType { VARIABLE, FUNCTION };

struct TableEntry {
    IdentifierType Type;
    int64_t StackOffset = 0;
};

class ScopeStack {
  public:
    void Insert(const std::string& name, const TableEntry& entry);
    const TableEntry& Lookup(const std::string& name) const;
    void Print() const;

    void EnterScope();
    size_t ExitScope();

  private:
    std::vector<std::unordered_map<std::string, TableEntry>> m_Scopes;
};

} // namespace Compiler