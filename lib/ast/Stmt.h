#pragma once

#include <ranges>

#include "AstNode.h"

namespace ast {

class BlockStatement : public Stmt {
public:
   BlockStatement(
      BumpAllocator& alloc, 
      array_ref<Stmt*> stmts
   ) noexcept :
      stmts_{stmts} {
         stmts_.reserve(stmts_.size());
         stmts_.insert(stmts_.end(),
            std::make_move_iterator(stmts_.begin()),
            std::make_move_iterator(stmts_.end()));
      }

   auto stmts() const { return std::views::all(stmts_); }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

private:
   pmr_vector<Stmt*> stmts_;
};

class DeclStmt : public Stmt {
public:
   DeclStmt(VarDecl* decl) : decl_{decl} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto decl() const { return decl_; }

private:
   VarDecl* decl_;
};

class ExprStmt : public Stmt {
public:
   ExprStmt(Expr* expr) : expr_{expr} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto expr() const { return expr_; }

private:
   Expr* expr_;
};

class IfStmt : public Stmt {
public:
   IfStmt(Expr* condition, Stmt* thenStmt, Stmt* elseStmt) 
      : condition_{condition}, thenStmt_{thenStmt}, elseStmt_{elseStmt} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto condition() const { return condition_; }
   auto thenStmt() const { return thenStmt_; }
   auto elseStmt() const { return elseStmt_; }

private:
   Expr* condition_;
   Stmt* thenStmt_;
   Stmt* elseStmt_;
};

class WhileStmt : public Stmt {
public:
   WhileStmt(Expr* condition, Stmt* body) : condition_{condition}, body_{body} {}
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto condition() const { return condition_; }
   auto body() const { return body_; }

private:
   Expr* condition_;
   Stmt* body_;
};

class ForStmt : public Stmt {
public:
   ForStmt(Stmt* init, Expr* condition, Stmt* update, Stmt* body) 
      : init_{init}, condition_{condition}, update_{update}, body_{body} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   
   auto init() const { return init_; }
   auto condition() const { return condition_; }
   auto update() const { return update_; }
   auto body() const { return body_; }
   
private:
   Stmt* init_;
   Expr* condition_;
   Stmt* update_;
   Stmt* body_;
};

class ReturnStmt : public Stmt {
public:
   ReturnStmt(Expr* expr) : expr_{expr} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto expr() const { return expr_; }

private:
   Expr* expr_;
};

class NullStmt : public Stmt {
public:
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
};

} // namespace ast
