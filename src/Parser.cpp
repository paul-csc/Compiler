#include "pch.h"
#include "Parser.h"

namespace Compiler {

Parser::Parser(const std::vector<Token>& tokens)
    : m_Tokens(tokens), m_Index(0), m_Allocator(4 * 1024 * 1024) {} // 4 MB

Program* Parser::ParseProgram() {
    m_Index = 0;
    Program* program = m_Allocator.alloc<Program>();

    while (Peek().Type != END_OF_FILE) {
        program->Statements.push_back(ParseStatement());
    }

    return program;
}

Term* Parser::ParseTerm() {
    const Token& tok = Peek();
    if (tok.Type == END_OF_FILE) {
        Error(m_Tokens.back().Location, "Expected term");
    }

    if (auto t = Match(LITERAL)) {
        TermLiteral* term = m_Allocator.alloc<TermLiteral>(*t->Value);
        return m_Allocator.alloc<Term>(term);
    } else if (auto t = Match(IDENTIFIER)) {
        TermIdentifier* term = m_Allocator.alloc<TermIdentifier>(*t->Value);
        return m_Allocator.alloc<Term>(term);
    }

    if (Match(L_PAREN)) {
        Expression* expr = ParseExpression();
        Expect(R_PAREN);
        TermParen* term = m_Allocator.alloc<TermParen>(expr);
        return m_Allocator.alloc<Term>(term);
    }

    Error(tok.Location, "Unexpected token in term");
    return nullptr; // never reached
}

Expression* Parser::ParseExpression(int minPrecedence) {
    Expression* left = m_Allocator.alloc<Expression>(ParseTerm());

    while (true) {
        const Token& opTok = Peek();

        auto prec = GetPrecedence(opTok.Type);
        if (!prec || *prec < minPrecedence) {
            break;
        }
        Consume();

        Expression* right = ParseExpression(*prec + 1);
        ExprBinary* bin = m_Allocator.alloc<ExprBinary>(ToStr(opTok.Type)[0], left, right);
        left = m_Allocator.alloc<Expression>(bin);
    }

    return left;
}

Statement* Parser::ParseStatement() {
    if (Match(EXIT)) {
        Expression* expr = ParseExpression();
        Expect(SEMI);

        StmtExit* stmt = m_Allocator.alloc<StmtExit>(expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(VAR)) {
        std::string name = *Expect(IDENTIFIER).Value;

        Expect(EQUAL);
        Expression* expr = ParseExpression();
        Expect(SEMI);

        StmtDeclar* stmt = m_Allocator.alloc<StmtDeclar>(name, expr);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(IF)) {
        Expect(L_PAREN);        
        Expression* expr = ParseExpression();
        Expect(R_PAREN);
        Expect(L_BRACE);
        Scope* scope = ParseScope();

        StmtIf* stmt = m_Allocator.alloc<StmtIf>(expr, scope);
        return m_Allocator.alloc<Statement>(stmt);
    } else if (Match(L_BRACE)) {
        Scope* scope = ParseScope();
        return m_Allocator.alloc<Statement>(scope);
    } else if (Match(R_BRACE)) {
        return nullptr; // special tag for scope end
    }

    std::string name = *Expect(IDENTIFIER).Value;

    Expect(EQUAL);
    Expression* expr = ParseExpression();
    Expect(SEMI);

    StmtAssign* stmt = m_Allocator.alloc<StmtAssign>(name, expr);
    return m_Allocator.alloc<Statement>(stmt);
}

Scope* Parser::ParseScope() {
    Scope* scope = m_Allocator.alloc<Scope>();
    while (Statement* stmt = ParseStatement()) {
        scope->Statements.push_back(stmt);
    }
    return scope;
}

} // namespace Compiler
