#include "passes/CompilerPasses.h"

#include <memory>
#include <string_view>

#include "ast/DeclContext.h"
#include "diagnostics/Diagnostics.h"
#include "diagnostics/Location.h"
#include "diagnostics/SourceManager.h"
#include "grammar/Joos1WGrammar.h"
#include "parsetree/ParseTreeVisitor.h"
#include "semantic/HierarchyChecker.h"
#include "semantic/NameResolver.h"
#include "semantic/Semantic.h"
#include "third-party/CLI11.h"
#include "utils/BumpAllocator.h"
#include "utils/PassManager.h"
#include "utils/Utils.h"

using std::string_view;
using utils::Pass;
using utils::PassManager;

namespace joos1 {

/* ===--------------------------------------------------------------------=== */
// Joos1WParserPass
/* ===--------------------------------------------------------------------=== */

void Joos1WParserPass::Run() {
   // Print the file being parsed if verbose
   if(PM().Diag().Verbose()) {
      auto os = PM().Diag().ReportDebug();
      os << "Parsing file ";
      SourceManager::print(os.get(), file_);
   }
   // Check for non-ASCII characters
   checkNonAscii(SourceManager::getBuffer(file_));
   // Parse the file
   BumpAllocator alloc{NewHeap()};
   Joos1WParser parser{file_, alloc, &PM().Diag()};
   int result = parser.parse(tree_);
   // If no parse tree was generated, report error if not already reported
   if((result != 0 || !tree_) && !PM().Diag().hasErrors())
      PM().Diag().ReportError(SourceRange{file_}) << "failed to parse file";
   if(result != 0 || !tree_) return;
   // If the parse tree is poisoned, report error
   if(tree_->is_poisoned()) {
      PM().Diag().ReportError(SourceRange{file_}) << "parse tree is poisoned";
      return;
   }
   // If the parse tree has invalid literal types, report error
   if(!isLiteralTypeValid(tree_)) {
      PM().Diag().ReportError(SourceRange{file_})
            << "invalid literal types in parse tree";
      return;
   }
}

bool Joos1WParserPass::isLiteralTypeValid(parsetree::Node* node) {
   if(node == nullptr) return true;
   if(node->get_node_type() == parsetree::Node::Type::Literal)
      return static_cast<parsetree::Literal*>(node)->isValid();
   for(size_t i = 0; i < node->num_children(); i++) {
      if(!isLiteralTypeValid(node->child(i))) return false;
   }
   return true;
}

void Joos1WParserPass::checkNonAscii(std::string_view str) {
   for(unsigned i = 0; i < str.length(); i++) {
      if(static_cast<unsigned char>(str[i]) > 127) {
         PM().Diag().ReportError(SourceRange{file_})
               << "non-ASCII character in file";
         return;
      }
   }
}

/* ===--------------------------------------------------------------------=== */
// AstContextPass
/* ===--------------------------------------------------------------------=== */

void AstContextPass::Init() {
   // Grab a long-living heap
   alloc = std::make_unique<BumpAllocator>(NewHeap());
}

void AstContextPass::Run() {
   sema = std::make_unique<ast::Semantic>(*alloc, PM().Diag());
}

AstContextPass::~AstContextPass() {
   // FIXME(kevin): This is a really bad bug that occurs in many places
   sema.reset();
   alloc.reset();
}

/* ===--------------------------------------------------------------------=== */
// AstBuilderPass
/* ===--------------------------------------------------------------------=== */

/// @brief Prints the parse tree back to the parent node
/// @param node Node to trace back to the parent
static inline void trace_node(parsetree::Node const* node, std::ostream& os) {
   if(node->parent() != nullptr) {
      trace_node(node->parent(), os);
      os << " -> ";
   }
   os << node->type_string() << std::endl;
}

/// @brief Marks a node and all its parents
/// @param node The node to mark
static inline void mark_node(parsetree::Node* node) {
   if(!node) return;
   mark_node(node->parent());
   node->mark();
}

AstBuilderPass::AstBuilderPass(PassManager& PM, Joos1WParserPass& dep) noexcept
      : Pass(PM), dep{dep} {}

void AstBuilderPass::Init() {
   optCheckName = PM().PO().GetExistingOption("--enable-filename-check");
}

void AstBuilderPass::Run() {
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
      PM().Diag().ReportError(SourceRange{}) << "ParseTreeException occured";
      std::cerr << "ParseTreeException: " << e.what() << " in file ";
      SourceManager::print(std::cerr, dep.File());
      std::cerr << std::endl;
      std::cerr << "Parse tree trace:" << std::endl;
      trace_node(e.get_where(), std::cerr);
      return;
   }
   if(cu_ == nullptr && !PM().Diag().hasErrors()) {
      PM().Diag().ReportError(PT->location()) << "failed to build AST";
      return;
   }
   // Check if the file name matches the class name
   bool shouldCheck = optCheckName && optCheckName->as<bool>();
   std::pmr::string fileName{SourceManager::getFileName(dep.File()), alloc};
   if(!fileName.empty() && shouldCheck) {
      auto cuBody = cu_->bodyAsDecl();
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

/* ===--------------------------------------------------------------------=== */
// LinkerPass
/* ===--------------------------------------------------------------------=== */

void LinkerPass::Run() {
   std::pmr::vector<ast::CompilationUnit*> cus{};
   // Get the semantic analysis
   auto& sema = GetPass<AstContextPass>().Sema();
   // Create the linking unit
   for(auto* pass : GetPasses<AstBuilderPass>())
      cus.push_back(pass->CompilationUnit());
   lu_ = sema.BuildLinkingUnit(cus);
}

/* ===--------------------------------------------------------------------=== */
// HierarchyCheckerPass
/* ===--------------------------------------------------------------------=== */

void HierarchyCheckerPass::Run() {
   auto lu = GetPass<LinkerPass>().LinkingUnit();
   checker.Check(lu);
   for(auto* cu : lu->compliationUnits()) {
      auto* classDecl = dyn_cast_or_null<ast::ClassDecl>(cu->body());
      if(!classDecl) continue;
      // Check for each class in the LU, the super classes have a default ctor
      for(auto* super : classDecl->superClasses()) {
         if(!super) continue;
         if(!dyn_cast_or_null<ast::ClassDecl>(super->decl())) continue;
         if(cast<ast::ClassDecl>(super->decl())->hasDefaultCtor()) continue;
         PM().Diag().ReportError(super->location())
               << "super class "
               << (super->decl()->hasCanonicalName()
                         ? super->decl()->getCanonicalName()
                         : super->decl()->name())
               << " of "
               << (classDecl->hasCanonicalName() ? classDecl->getCanonicalName()
                                                 : classDecl->name())
               << " does not have a default constructor";
         break;
      }
   }
}

/* ===--------------------------------------------------------------------=== */
// NameResolverPass
/* ===--------------------------------------------------------------------=== */

void NameResolverPass::Init() {
   alloc = std::make_unique<BumpAllocator>(NewHeap());
}

void NameResolverPass::Run() {
   auto lu = GetPass<LinkerPass>().LinkingUnit();
   auto sema = &GetPass<AstContextPass>().Sema();
   NR = std::make_unique<semantic::NameResolver>(*alloc, PM().Diag());
   NR->Init(lu, sema);
   if(PM().Diag().hasErrors()) return;
   resolveRecursive(lu);
}

void NameResolverPass::replaceObjectClass(ast::AstNode* node) {
   auto decl = dyn_cast_or_null<ast::ClassDecl*>(node);
   if(!decl) return;
   // Check if the class is Object
   if(decl != NR->GetJavaLang().Object) return;
   // Go through the superclasses and replace Object with nullptr
   for(int i = 0; i < 2; i++) {
      auto super = decl->superClasses()[i];
      if(!super) continue;
      // If the superclass is not resolved, then we should just bail out
      if(!PM().Diag().hasErrors())
         assert(super->isResolved() && "Superclass should be resolved");
      else
         continue;
      // Do not allow Object to extend Object
      if(super->decl() == NR->GetJavaLang().Object)
         decl->mut_superClasses()[i] = nullptr;
   }
}

void NameResolverPass::resolveExpr(ast::Expr* expr) {
   if(!expr) return;
   for(auto node : expr->mut_nodes()) {
      auto tyNode = dyn_cast<ast::exprnode::TypeNode>(node);
      if(!tyNode) continue;
      if(tyNode->isTypeResolved()) continue;
      tyNode->resolveUnderlyingType(*NR);
   }
}

void NameResolverPass::resolveRecursive(ast::AstNode* node) {
   assert(node && "Node must not be null here!");
   for(auto child : node->mut_children()) {
      if(!child) continue;
      if(auto cu = dyn_cast<ast::CompilationUnit*>(child)) {
         // If the CU has no body, then we can skip to the next CU :)
         if(!cu->body()) return;
         // Resolve the current compilation unit's body
         NR->BeginContext(cu);
         resolveRecursive(cu->mut_body());
         replaceObjectClass(cu->mut_body());
         NR->EndContext();
      } else if(auto ty = dyn_cast<ast::Type*>(child)) {
         if(ty->isInvalid()) continue;
         // If the type is not resolved, then we should resolve it
         if(!ty->isResolved()) ty->resolve(*NR);
      } else {
         // Resolve any Type in expressions
         if(auto decl = dyn_cast<ast::TypedDecl*>(child)) {
            resolveExpr(decl->mut_init());
         } else if(auto stmt = dyn_cast<ast::Stmt*>(child)) {
            for(auto expr : stmt->mut_exprs()) resolveExpr(expr);
         }
         // This is a generic node, just resolve its children
         resolveRecursive(child);
      }
   }
}

NameResolverPass::~NameResolverPass() {
   // FIXME(kevin): This is a really bad bug that occurs in many places
   NR.reset();
   alloc.reset();
}

/* ===--------------------------------------------------------------------=== */
// PrintASTPass
/* ===--------------------------------------------------------------------=== */

class PrintASTPass final : public Pass {
public:
   PrintASTPass(PassManager& PM) noexcept : Pass(PM) {}
   string_view Name() const override { return "print-ast"; }
   string_view Desc() const override { return "Print AST"; }

   void Init() override {
      optDot = PM().PO().GetExistingOption("--print-dot");
      optOutput = PM().PO().GetExistingOption("--print-output");
      optSplit = PM().PO().GetExistingOption("--print-split");
      optIgnoreStd = PM().PO().GetExistingOption("--print-ignore-std");
   }

   void Run() override {
      // 1a. Grab the AST node
      auto& pass = GetPass<LinkerPass>();
      auto* node = pass.LinkingUnit();
      if(!node) return;
      // 1b. Create a new heap to build a new AST if we're ignoring stdlib
      BumpAllocator alloc{NewHeap()};
      ast::Semantic newSema{alloc, PM().Diag()};
      if(optIgnoreStd && optIgnoreStd->count()) {
         std::pmr::vector<ast::CompilationUnit*> cus{alloc};
         for(auto* cu : node->compliationUnits())
            if(!cu->isStdLib()) cus.push_back(cu);
         node = newSema.BuildLinkingUnit(cus);
      }
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
   CLI::Option *optDot, *optOutput, *optSplit, *optIgnoreStd;
};

} // namespace joos1

/* ===--------------------------------------------------------------------=== */
// Register the passes
/* ===--------------------------------------------------------------------=== */

REGISTER_PASS_NS(joos1, AstContextPass);
REGISTER_PASS_NS(joos1, LinkerPass);
REGISTER_PASS_NS(joos1, NameResolverPass);
REGISTER_PASS_NS(joos1, HierarchyCheckerPass);
REGISTER_PASS_NS(joos1, PrintASTPass);

Pass& NewJoos1WParserPass(PassManager& PM, SourceFile file, Pass* prev) {
   using namespace joos1;
   return PM.AddPass<Joos1WParserPass>(file, prev);
}

Pass& NewAstBuilderPass(PassManager& PM, Pass* depends) {
   using namespace joos1;
   // "depends" must be a Joos1WParserPass
   auto* p = cast<Joos1WParserPass*>(depends);
   return PM.AddPass<AstBuilderPass>(*p);
}
