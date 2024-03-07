#pragma once

#include <ranges>

#include "AstNode.h"
#include "Decl.h"
#include "diagnostics/Location.h"
#include "utils/Utils.h"

namespace ast {

class VarDecl;

class BlockStatement final : public Stmt {
public:
   BlockStatement(BumpAllocator& alloc, array_ref<Stmt*> stmts) noexcept
         : stmts_{alloc} {
      utils::move_vector<Stmt*>(stmts, stmts_);
   }

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   utils::Generator<ast::AstNode const*> children() const override {
      for(auto stmt : stmts_) co_yield stmt;
   }
   utils::Generator<const Expr*> exprs() const override { co_yield nullptr; }
   auto stmts() const { return std::views::all(stmts_); }

private:
   pmr_vector<Stmt*> stmts_;
};

class DeclStmt final : public Stmt {
public:
   DeclStmt(VarDecl* decl) : decl_{decl} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto decl() const { return decl_; }
   utils::Generator<ast::AstNode const*> children() const override {
      co_yield decl_;
   }
   utils::Generator<const Expr*> exprs() const override { co_yield nullptr; }

private:
   VarDecl* decl_;
};

class ExprStmt final : public Stmt {
public:
   ExprStmt(Expr* expr) : expr_{expr} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto expr() const { return expr_; }
   utils::Generator<const Expr*> exprs() const override { co_yield expr_; }

private:
   Expr* expr_;
};

class IfStmt final : public Stmt {
public:
   IfStmt(Expr* condition, Stmt* thenStmt, Stmt* elseStmt)
         : condition_{condition}, thenStmt_{thenStmt}, elseStmt_{elseStmt} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto condition() const { return condition_; }
   auto thenStmt() const { return thenStmt_; }
   auto elseStmt() const { return elseStmt_; }
   utils::Generator<const Expr*> exprs() const override { co_yield condition_; }
   utils::Generator<ast::AstNode const*> children() const override {
      co_yield thenStmt_;
      if(elseStmt_) co_yield elseStmt_;
   }

private:
   Expr* condition_;
   Stmt* thenStmt_;
   Stmt* elseStmt_;
};

class WhileStmt final : public Stmt {
public:
   WhileStmt(Expr* condition, Stmt* body) : condition_{condition}, body_{body} {}
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;
   auto condition() const { return condition_; }
   auto body() const { return body_; }
   utils::Generator<const Expr*> exprs() const override { co_yield condition_; }
   utils::Generator<ast::AstNode const*> children() const override {
      co_yield body_;
   }

private:
   Expr* condition_;
   Stmt* body_;
};

class ForStmt final : public Stmt {
public:
   ForStmt(Stmt* init, Expr* condition, Stmt* update, Stmt* body)
         : init_{init}, condition_{condition}, update_{update}, body_{body} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto init() const { return init_; }
   auto condition() const { return condition_; }
   auto update() const { return update_; }
   auto body() const { return body_; }
   utils::Generator<const Expr*> exprs() const override { co_yield condition_; }
   utils::Generator<ast::AstNode const*> children() const override {
      co_yield init_;
      co_yield update_;
      co_yield body_;
   }

private:
   Stmt* init_;
   Expr* condition_;
   Stmt* update_;
   Stmt* body_;
};

class ReturnStmt final : public Stmt {
public:
   ReturnStmt(Expr* expr) : expr_{expr} {}

   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   auto expr() const { return expr_; }
   utils::Generator<const Expr*> exprs() const override { co_yield expr_; }

private:
   Expr* expr_;
};

class NullStmt final : public Stmt {
public:
   std::ostream& print(std::ostream& os, int indentation = 0) const override;
   int printDotNode(DotPrinter& dp) const override;

   utils::Generator<const Expr*> exprs() const override { co_yield nullptr; }
};

} // namespace ast
