#include "symbol_table.h"
#include "utils.h"
#include <iostream>

namespace Compiler {
    
void ScopeStack::EnterScope() {
    m_Scopes.emplace_back();
}

size_t ScopeStack::ExitScope() {
    if (m_Scopes.empty()) {
        Error("Attempted to exit scope with empty scope stack");
    }
    size_t popCount = m_Scopes.back().size();
    m_Scopes.pop_back();
    return popCount;
}

void ScopeStack::Insert(const std::string& name, const TableEntry& entry) {
    if (m_Scopes.empty()) {
        Error("No active scope");
    } else if (!m_Scopes.back().emplace(name, entry).second) {
        Error("Redefinition of identifier: " + name);
    }
}

const TableEntry& ScopeStack::Lookup(const std::string& name) const {
    for (auto it = m_Scopes.rbegin(); it != m_Scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    Error("Undeclared identifier: " + name);
}

void ScopeStack::Print() const {
    for (const auto& scope : m_Scopes) {
        for (const auto& [key, value] : scope) {
            std::cout << key << ":  type: " << value.Type << ", stackOffset: " << value.StackOffset << "\n";
        }
    }
}

} // namespace Compiler
