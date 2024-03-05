#include "CompilerPasses.h"

#include <memory>
#include <string_view>

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
      : Pass(PM), dep{dep} {
   optCheckName = PM.PO().GetExistingOption("--check-file-name");
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
   bool shouldCheck = optCheckName && optCheckName->as<bool>();
   auto fileName = SourceManager::getFileName(dep.File());
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
}

} // namespace joos1

/* ===--------------------------------------------------------------------=== */
// Register the passes
/* ===--------------------------------------------------------------------=== */

REGISTER_PASS_NS(joos1, AstContextPass);
REGISTER_PASS_NS(joos1, LinkerPass);
REGISTER_PASS_NS(joos1, NameResolverPass);
REGISTER_PASS_NS(joos1, HierarchyCheckerPass);

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
