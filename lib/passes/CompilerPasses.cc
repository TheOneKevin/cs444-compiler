#include <filesystem>

#include "diagnostics/Location.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"
#include "semantic/ExprResolver.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "semantic/TypeChecker.h"
#include "utils/CLI11.h"
#include "utils/PassManager.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

/* ===--------------------------------------------------------------------=== */
// Joos1WParserPass
/* ===--------------------------------------------------------------------=== */

class Joos1WParserPass final : public Pass {
public:
   Joos1WParserPass(PassManager& PM, SourceFile file, Pass* prev) noexcept
         : Pass(PM), file_{file}, prev_{prev} {}
   string_view Desc() const override { return "Joos1W Lexing and Parsing"; }
   void Run() override {
      // Print the file being parsed if verbose
      if(PM().Diag().Verbose()) {
         auto os = PM().Diag().ReportDebug();
         os << "Parsing file ";
         SourceManager::print(os.get(), file_);
      }
      // Check for non-ASCII characters
      // FIXME(kevin):
      /*for(unsigned i = 0; i < str.length(); i++) {
         if(static_cast<unsigned char>(str[i]) > 127) {
            std::cerr << "Parse error: non-ASCII character in input" << std::endl;
            return 42;
         }
      }*/
      // Parse the file
      BumpAllocator alloc{NewHeap()};
      Joos1WParser parser{file_, alloc, &PM().Diag()};
      int result = parser.parse(tree_);
      // If no parse tree was generated, report error if not already reported
      if((result != 0 || !tree_) && !PM().Diag().hasErrors()) {
         PM().Diag().ReportError(SourceRange{file_}) << "failed to parse file";
      }
      // If the parse tree is poisoned, report error
      if(tree_->is_poisoned()) {
         PM().Diag().ReportError(SourceRange{file_})
               << "parse tree is poisoned";
      }
      // If the parse tree has invalid literal types, report error
      if(!isLiteralTypeValid(tree_)) {
         PM().Diag().ReportError(SourceRange{file_})
               << "invalid literal types in parse tree";
      }
   }
   parsetree::Node* Tree() { return tree_; }
   SourceFile File() { return file_; }

private:
   /// @brief Checks if the literal type is valid recursively
   bool isLiteralTypeValid(parsetree::Node* node) {
      if(node == nullptr) return true;
      if(node->get_node_type() == parsetree::Node::Type::Literal)
         return static_cast<parsetree::Literal*>(node)->isValid();
      for(size_t i = 0; i < node->num_children(); i++) {
         if(!isLiteralTypeValid(node->child(i))) return false;
      }
      return true;
   }

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
   AstContextPass(PassManager& PM) noexcept
         : Pass(PM), alloc_{NewHeap()}, sema_{alloc_, PM.Diag()} {}
   string_view Desc() const override { return "AST Context Lifetime"; }
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
   AstBuilderPass(PassManager& PM, Joos1WParserPass& dep) noexcept
         : Pass(PM), dep{dep} {
      /*optCheckName =
            PM.PO().AddFlag("--check-file-name",
                            "Check if the file name matches the class name");*/
   }
   string_view Desc() const override { return "ParseTree -> AST Building"; }
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
      // Check if the file name matches the class name
      // FIXME(kevin): optCheckName && optCheckName->as<bool>()
      if(true) {
         auto cuBody = cu_->bodyAsDecl();
         auto fileName = SourceManager::getFileName(dep.File());
         // Grab the file without the path and the extension
         fileName = fileName.substr(0, fileName.find_last_of('.'));
         fileName = fileName.substr(fileName.find_last_of('/') + 1);
         if(cuBody->name() != fileName) {
            PM().Diag().ReportError(cuBody->location())
                  << "class/interface name does not match file name: "
                  << cuBody->name() << " != " << fileName;
            return;
         }
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
   CLI::Option* optCheckName;
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
   LinkerPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Desc() const override { return "AST Linking"; }
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
   NameResolverPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-name"; }
   string_view Desc() const override { return "Name Resolution"; }
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
   HierarchyCheckerPass(PassManager& PM) noexcept
         : Pass(PM), checker{semantic::HierarchyChecker(PM.Diag())} {}
   string_view Name() const override { return "sema-hier"; }
   string_view Desc() const override { return "Hierarchy Checking"; }
   void Run() override {
      auto lu = GetPass<LinkerPass>().LinkingUnit();
      checker.Check(lu);
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<LinkerPass>());
      ComputeDependency(GetPass<NameResolverPass>());
   }
   semantic::HierarchyChecker checker;
};

Pass& NewHierarchyCheckerPass(PassManager& PM) {
   return PM.AddPass<HierarchyCheckerPass>();
}

/* ===--------------------------------------------------------------------=== */
// PrintASTPass
/* ===--------------------------------------------------------------------=== */

class PrintASTPass final : public Pass {
public:
   PrintASTPass(PassManager& PM) noexcept : Pass(PM) {
      optDot = PM.PO().GetExistingOption("--print-dot");
      optOutput = PM.PO().GetExistingOption("--print-output");
      optSplit = PM.PO().GetExistingOption("--print-split");
   }
   string_view Name() const override { return "print-ast"; }
   string_view Desc() const override { return "Print AST"; }

   void Run() override {
      // 1. Grab the AST node
      auto& pass = GetPass<LinkerPass>();
      auto* node = pass.LinkingUnit();
      if(!node) return;
      // 2. Interpret the command line options
      bool printDot = optDot && optDot->count();
      bool printSplit = optSplit && optSplit->count();
      std::string outputPath = optOutput ? optOutput->as<std::string>() : "";
      // 3. Print the AST depending on the options
      if(!printSplit) {
         // 1. Now we try to open the output file if exists
         std::ofstream file{};
         if(!outputPath.empty()) {
            file = std::ofstream{outputPath};
            if(!file.is_open()) {
               PM().Diag().ReportError(SourceRange{})
                     << "failed to open output file " << outputPath;
               return;
            }
         }
         // 2. Set the output stream
         std::ostream& os = file.is_open() ? file : std::cout;
         // 3. Now we can print the AST
         if(printDot)
            node->printDot(os);
         else
            node->print(os);
      } else {
         namespace fs = std::filesystem;
         // 1. Create the output directory if it doesn't exist
         fs::create_directories(outputPath);
         // 2. Create a new file for each compilation unit
         for(auto const& cu : node->compliationUnits()) {
            if(!cu->body() || !cu->bodyAsDecl()->hasCanonicalName()) continue;
            std::string canonName{cu->bodyAsDecl()->getCanonicalName()};
            auto filepath = fs::path{outputPath} / fs::path{canonName + ".dot"};
            std::ofstream file{filepath};
            if(!file.is_open()) {
               PM().Diag().ReportError(SourceRange{})
                     << "failed to open output file " << filepath.string();
               return;
            }
            // 3. Set the output stream
            std::ostream& os = file;
            // 4. Now we can print the AST
            cu->printDot(os);
         }
      }
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<LinkerPass>());
      ComputeDependency(GetPass<AstContextPass>());
   }
   CLI::Option *optDot, *optOutput, *optSplit;
};

Pass& NewPrintASTPass(PassManager& PM) { return PM.AddPass<PrintASTPass>(); }

/* ===--------------------------------------------------------------------=== */
// ExprResolverPass
/* ===--------------------------------------------------------------------=== */

class ExprResolverPass final : public Pass {
public:
   ExprResolverPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-expr"; }
   string_view Desc() const override { return "Expression Resolution"; }
   void Run() override {
      auto lu = GetPass<LinkerPass>().LinkingUnit();
      // TODO(kevin):
      // semantic::ExprResolver resolver{PM().Diag(), lu};
      // resolver.Resolve();
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<NameResolverPass>());
   }
};

Pass& NewExprResolverPass(PassManager& PM) {
   return PM.AddPass<ExprResolverPass>();
}

/* ===--------------------------------------------------------------------=== */
// TypeCheckerPass
/* ===--------------------------------------------------------------------=== */

class TypeCheckerPass final : public Pass {
public:
   TypeCheckerPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "sema-type"; }
   string_view Desc() const override { return "Type Checking"; }
   void Run() override {
      auto lu = GetPass<LinkerPass>().LinkingUnit();
      semantic::TypeChecker typeChecker{PM().Diag()};
      typeChecker.check(lu);
   }

private:
   void computeDependencies() override {
      ComputeDependency(GetPass<AstContextPass>());
      ComputeDependency(GetPass<ExprResolverPass>());
      ComputeDependency(GetPass<LinkerPass>());
   }
};

Pass& NewTypeCheckerPass(PassManager& PM) { return PM.AddPass<TypeCheckerPass>(); }
