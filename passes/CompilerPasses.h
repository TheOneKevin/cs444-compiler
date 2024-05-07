#pragma once

#include <string_view>

#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "utils/PassManager.h"

namespace joos1 {

using std::string_view;
using utils::Pass;
using utils::PassManager;

/* ===--------------------------------------------------------------------=== */

class Joos1WParserPass final : public Pass {
public:
   Joos1WParserPass(PassManager& PM, SourceFile file, Pass* prev) noexcept
         : Pass(PM), file_{file}, prev_{prev} {}
   string_view Desc() const override { return "Joos1W Lexing and Parsing"; }
   void Run() override;
   parsetree::Node* Tree() { return tree_; }
   SourceFile File() { return file_; }

private:
   /// @brief Checks if the literal type is valid recursively
   bool isLiteralTypeValid(parsetree::Node* node);
   void checkNonAscii(std::string_view str);

private:
   void ComputeDependencies() override {
      if(prev_) AddDependency(*prev_);
   }
   SourceFile file_;
   parsetree::Node* tree_;
   Pass* prev_;
};

/* ===--------------------------------------------------------------------=== */

class AstContextPass final : public Pass {
public:
   AstContextPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Desc() const override { return "AST Context Lifetime"; }
   void Run() override;
   ast::Semantic& Sema() { return *sema; }

private:
   void ComputeDependencies() override {}
   std::unique_ptr<ast::Semantic> sema;
};

/* ===--------------------------------------------------------------------=== */

class AstBuilderPass final : public Pass {
public:
   AstBuilderPass(PassManager& PM, Joos1WParserPass& dep) noexcept;
   string_view Desc() const override { return "ParseTree -> AST Building"; }
   void Init() override;
   void Run() override;
   ast::CompilationUnit* CompilationUnit() { return cu_; }

private:
   void ComputeDependencies() override {
      AddDependency(dep);
      AddDependency(GetPass<AstContextPass>());
   }
   ast::CompilationUnit* cu_;
   Joos1WParserPass& dep;
   CLI::Option* optCheckName;
};

/* ===--------------------------------------------------------------------=== */

class LinkerPass final : public Pass {
public:
   LinkerPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Desc() const override { return "AST Linking"; }
   void Run() override;
   ast::LinkingUnit* LinkingUnit() { return lu_; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContextPass>());
      for(auto* pass : GetPasses<AstBuilderPass>()) AddDependency(*pass);
   }
   ast::LinkingUnit* lu_;
};

/* ===--------------------------------------------------------------------=== */

class NameResolverPass final : public Pass {
public:
   NameResolverPass(PassManager& PM) noexcept : Pass{PM} {}
   string_view Name() const override { return "sema-name"; }
   string_view Desc() const override { return "Name Resolution"; }
   void Run() override;
   semantic::NameResolver& Resolver() { return *NR; }

private:
   void replaceObjectClass(ast::AstNode* node);
   void resolveExpr(ast::Expr* expr);
   void resolveRecursive(ast::AstNode* node);

   void ComputeDependencies() override {
      AddDependency(GetPass<AstContextPass>());
      AddDependency(GetPass<LinkerPass>());
   }

   std::unique_ptr<semantic::NameResolver> NR;
};

/* ===--------------------------------------------------------------------=== */

class HierarchyCheckerPass final : public Pass {
public:
   HierarchyCheckerPass(PassManager& PM) noexcept
         : Pass(PM), checker{semantic::HierarchyChecker(PM.Diag())} {}
   string_view Name() const override { return "sema-hier"; }
   string_view Desc() const override { return "Hierarchy Checking"; }
   void Run() override;
   semantic::HierarchyChecker& Checker() { return checker; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContextPass>());
      AddDependency(GetPass<LinkerPass>());
      AddDependency(GetPass<NameResolverPass>());
   }
   semantic::HierarchyChecker checker;
};

} // namespace joos1
