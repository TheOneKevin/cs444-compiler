#pragma once

#include <memory>
#include <string_view>

#include "AllPasses.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "utils/PassManager.h"

namespace passes::joos1 {

using std::string_view;
using utils::Pass;
using utils::PassManager;

/* ===--------------------------------------------------------------------=== */

class Parser final : public Pass {
public:
   Parser(PassManager& PM, SourceFile file, Pass* prev) noexcept
         : Pass(PM), file_{file}, prev_{prev} {}
   string_view Name() const override { return ""; }
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

class AstContext final : public Pass {
public:
   AstContext(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return ""; }
   string_view Desc() const override { return "AST Context Lifetime"; }
   void Run() override;
   ast::Semantic& Sema() { return *sema; }

private:
   void ComputeDependencies() override {}
   std::unique_ptr<ast::Semantic> sema;
};

/* ===--------------------------------------------------------------------=== */

class AstBuilder final : public Pass {
public:
   AstBuilder(PassManager& PM, Parser& dep) noexcept;
   string_view Name() const override { return ""; }
   string_view Desc() const override { return "ParseTree -> AST Building"; }
   void Init() override;
   void Run() override;
   ast::CompilationUnit* CompilationUnit() { return cu_; }

private:
   void ComputeDependencies() override {
      AddDependency(dep);
      AddDependency(GetPass<AstContext>());
   }
   ast::CompilationUnit* cu_;
   Parser& dep;
   CLI::Option* optCheckName;
};

/* ===--------------------------------------------------------------------=== */

class Linker final : public Pass {
public:
   Linker(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return ""; }
   string_view Desc() const override { return "AST Linking"; }
   void Run() override;
   ast::LinkingUnit* LinkingUnit() { return lu_; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContext>());
      for(auto* pass : GetPasses<AstBuilder>()) AddDependency(*pass);
   }
   ast::LinkingUnit* lu_;
};

/* ===--------------------------------------------------------------------=== */

class NameResolver final : public Pass {
public:
   NameResolver(PassManager& PM) noexcept : Pass{PM} {}
   string_view Name() const override { return "sema-name"; }
   string_view Desc() const override { return "Name Resolution"; }
   int Tag() const override { return static_cast<int>(PassTag::FrontendPass); }
   void Run() override;
   void GC() override { NR = nullptr; }
   semantic::NameResolver& Resolver() { return *NR; }

private:
   void replaceObjectClass(ast::AstNode* node);
   void resolveExpr(ast::Expr* expr);
   void resolveRecursive(ast::AstNode* node);
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContext>());
      AddDependency(GetPass<Linker>());
   }
   std::unique_ptr<semantic::NameResolver> NR;
};

/* ===--------------------------------------------------------------------=== */

class HierarchyChecker final : public Pass {
public:
   HierarchyChecker(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-hier"; }
   string_view Desc() const override { return "Hierarchy Checking"; }
   int Tag() const override { return static_cast<int>(PassTag::FrontendPass); }
   void Run() override;
   void GC() override { HC = nullptr; }
   semantic::HierarchyChecker& Checker() { return *HC; }

private:
   void ComputeDependencies() override {
      AddDependency(GetPass<AstContext>());
      AddDependency(GetPass<Linker>());
      AddDependency(GetPass<NameResolver>());
   }
   std::unique_ptr<semantic::HierarchyChecker> HC;
};

} // namespace passes::joos1
