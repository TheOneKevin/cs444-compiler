#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

/* ===--------------------------------------------------------------------=== */
// Joos1WParserPass
/* ===--------------------------------------------------------------------=== */

class Joos1WParserPass final : public Pass {
public:
   Joos1WParserPass(PassManager& pm, SourceFile file, Pass* prev) noexcept
         : Pass(pm), file_{file}, prev_{prev} {}
   string_view Name() const override { return "Joos1W Lexing and Parsing"; }
   void Run() override {
      // Print the file being parsed if verbose
      if(PM().Diag().Verbose()) {
         auto os = PM().Diag().ReportDebug();
         os << "Parsing file ";
         SourceManager::print(os.get(), file_);
      }
      // Parse the file
      BumpAllocator alloc{NewHeap()};
      Joos1WParser parser{file_, alloc, &PM().Diag()};
      int result = parser.parse(tree_);
      // If no parse tree was generated, report error if not already reported
      if((result != 0 || !tree_) && !PM().Diag().hasErrors()) {
         PM().Diag().ReportError(SourceRange{file_}) << "failed to parse file";
      }
   }
   parsetree::Node* Tree() { return tree_; }
   SourceFile File() { return file_; }

private:
   void computeDependencies() override {
      if(prev_) ComputeDependency(*prev_);
   }
   SourceFile file_;
   parsetree::Node* tree_;
   Pass* prev_;
};

Pass& NewJoos1WParserPass(PassManager& PM, SourceFile file, Pass* prev) {
   return PM.AddPass<Joos1WParserPass>(file, prev);
}

/* ===--------------------------------------------------------------------=== */
// AstContextPass
/* ===--------------------------------------------------------------------=== */

class AstContextPass final : public Pass {
public:
   AstContextPass(PassManager& pm) noexcept
         : Pass(pm), alloc_{NewHeap()}, sema_{alloc_, PM().Diag()} {}
   string_view Name() const override { return "AST Context Lifetime"; }
   void Run() override {}
   ast::Semantic& Sema() { return sema_; }

private:
   void computeDependencies() override {}
   BumpAllocator alloc_;
   ast::Semantic sema_;
};

AstContextPass& NewAstContextPass(PassManager& PM) {
   return PM.AddPass<AstContextPass>();
}

/* ===--------------------------------------------------------------------=== */
// AstBuilderPass
/* ===--------------------------------------------------------------------=== */

class AstBuilderPass final : public Pass {
private:
   /// @brief Prints the parse tree back to the parent node
   /// @param node Node to trace back to the parent
   inline void trace_node(parsetree::Node const* node, std::ostream& os) {
      if(node->parent() != nullptr) {
         trace_node(node->parent(), os);
         os << " -> ";
      }
      os << node->type_string() << std::endl;
   }

   /// @brief Marks a node and all its parents
   /// @param node The node to mark
   inline void mark_node(parsetree::Node* node) {
      if(!node) return;
      mark_node(node->parent());
      node->mark();
   }

public:
   AstBuilderPass(PassManager& pm, Joos1WParserPass& dep) noexcept
         : Pass(pm), dep{dep} {}
   string_view Name() const override { return "ParseTree -> AST Building"; }
   void Run() override {
      // Get the parse tree and the semantic analysis
      auto& sema = GetPass<AstContextPass>().Sema();
      auto* PT = dep.Tree();
      // Create a new heap just for creating the AST
      BumpAllocator alloc{NewHeap()};
      parsetree::ParseTreeVisitor visitor{sema, alloc};
      // Store the result in the pass
      try {
         cu_ = visitor.visitCompilationUnit(PT);
      } catch(const parsetree::ParseTreeException& e) {
         std::cerr << "ParseTreeException: " << e.what() << " in file ";
         SourceManager::print(std::cerr, dep.File());
         std::cerr << std::endl;
         std::cerr << "Parse tree trace:" << std::endl;
         trace_node(e.get_where(), std::cerr);
      }
      if(cu_ == nullptr && !PM().Diag().hasErrors()) {
         PM().Diag().ReportError(PT->location()) << "failed to build AST";
      }
   }
   ast::CompilationUnit* CompilationUnit() { return cu_; }

private:
   void computeDependencies() override {
      ComputeDependency(dep);
      ComputeDependency(GetPass<AstContextPass>());
   }
   ast::CompilationUnit* cu_;
   Joos1WParserPass& dep;
};

Pass& NewAstBuilderPass(PassManager& PM, Pass* depends) {
   auto* p = dynamic_cast<Joos1WParserPass*>(depends);
   if(!p) throw std::invalid_argument{"depends must be a Joos1WParserPass"};
   return PM.AddPass<AstBuilderPass>(*p);
}

/* ===--------------------------------------------------------------------=== */
// LinkerPass
/* ===--------------------------------------------------------------------=== */

class LinkerPass final : public Pass {
public:
   LinkerPass(PassManager& pm) noexcept : Pass(pm) {}
   string_view Name() const override { return "AST Linking"; }
   void Run() override {
      // Grab a temporary heap
      BumpAllocator alloc{NewHeap()};
      std::pmr::vector<ast::CompilationUnit*> cus{alloc};
      // Get the semantic analysis
      auto& sema = GetPass<AstContextPass>().Sema();
      // Create the linking unit
      for(auto* pass : GetPasses<AstBuilderPass>())
         cus.push_back(pass->CompilationUnit());
      lu_ = sema.BuildLinkingUnit(cus);
   }
   ast::LinkingUnit* LinkingUnit() { return lu_; }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      for(auto* pass : GetPasses<AstBuilderPass>()) ComputeDependency(*pass);
   }
   ast::LinkingUnit* lu_;
};

Pass& NewLinkerPass(PassManager& PM) { return PM.AddPass<LinkerPass>(); }

/* ===--------------------------------------------------------------------=== */
// NameResolverPass
/* ===--------------------------------------------------------------------=== */

class NameResolverPass final : public Pass {
public:
   NameResolverPass(PassManager& pm) noexcept : Pass(pm) {}
   string_view Name() const override { return "Name Resolution"; }
   void Run() override {
      auto lu = GetPass<LinkerPass>().LinkingUnit();
      BumpAllocator alloc{NewHeap()};
      semantic::NameResolver resolver{alloc, PM().Diag(), lu};
      resolver.Resolve();
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<LinkerPass>());
   }
};

Pass& NewNameResolverPass(PassManager& PM) {
   return PM.AddPass<NameResolverPass>();
}

/* ===--------------------------------------------------------------------=== */
// HierarchyCheckerPass
/* ===--------------------------------------------------------------------=== */

class HierarchyCheckerPass final : public Pass {
public:
   HierarchyCheckerPass(PassManager& pm) noexcept : Pass(pm) {}
   string_view Name() const override { return "Hierarchy Checking"; }
   void Run() override {
      auto lu = GetPass<LinkerPass>().LinkingUnit();
      semantic::HierarchyChecker checker{PM().Diag(), lu};
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<LinkerPass>());
      ComputeDependency(GetPass<NameResolverPass>());
   }
};

Pass& NewHierarchyCheckerPass(PassManager& PM) {
   return PM.AddPass<HierarchyCheckerPass>();
}

/* ===--------------------------------------------------------------------=== */
// PrintASTPass
/* ===--------------------------------------------------------------------=== */

class PrintASTPass final : public Pass {
public:
   PrintASTPass(PassManager& pm, Pass* depends, std::ostream& os,
                bool dot) noexcept
         : Pass(pm), depends_{depends}, os{os}, dot{dot} {}
   string_view Name() const override { return "Print AST"; }
   void Run() override {
      ast::AstNode* node = nullptr;
      if(auto* pass = dynamic_cast<AstBuilderPass*>(depends_)) {
         node = pass->CompilationUnit();
      } else if(auto* pass = dynamic_cast<LinkerPass*>(depends_)) {
         node = pass->LinkingUnit();
      }
      if(!node) {
         return;
      }
      if(dot) {
         node->printDot(os);
      } else {
         node->print(os);
      }
   }

private:
   void computeDependencies() override {
      ComputeDependency(*depends_);
      ComputeDependency(GetPass<AstContextPass>());
   }
   Pass* depends_;
   std::ostream& os;
   bool dot;
};

Pass& NewPrintASTPass(PassManager& PM, Pass* depends, std::ostream& os, bool dot) {
   return PM.AddPass<PrintASTPass>(depends, os, dot);
}
