#include "semantic/ExprStaticChecker.h"

#include "ast/AST.h"
#include "ast/ExprNode.h"
#include "diagnostics/Diagnostics.h"

namespace semantic {

using ETy = ExprStaticCheckerData;
using ESC = ExprStaticChecker;
namespace ex = ast::exprnode;
using namespace diagnostics;

// FIXME(kevin): We seem to use this in a few places, maybe move to header?
static bool isDeclStatic(ast::Decl const* decl) {
   if(auto* field = dyn_cast<ast::FieldDecl>(decl)) {
      return field->modifiers().isStatic();
   } else if(auto* method = dyn_cast<ast::MethodDecl>(decl)) {
      return method->modifiers().isStatic();
   }
   return false;
}

void ESC::Evaluate(ast::Expr* expr, bool isStaticContext) {
   this->isStaticContext = isStaticContext;
   loc_ = expr->location();
   ETy single = this->EvaluateList(expr->list());
   // Handle the case of a single member access
   if(single.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
}

DiagnosticBuilder ESC::IllegalInstanceAccess() const {
   return diag.ReportError(loc_)
          << "cannot access or invoke instance members in a static context";
}

ETy ESC::mapValue(ExprValue& node) const {
   // If node is "this", reject immediately
   if(dyn_cast<ex::ThisNode>(node) && isStaticContext) {
      throw diag.ReportError(loc_) << "cannot use 'this' in a static context";
   }

   // 1. This node is a pure type node
   // 2. This node is a value node, with type and decl
   // 3. This node is a literal, with type but no decl
   assert(dyn_cast<ex::MethodName>(node) || node.isTypeResolved());
   if(dyn_cast<ex::LiteralNode>(node)) {
      return ETy{nullptr, node.type(), true, false};
   } else if(dyn_cast<ex::TypeNode>(node)) {
      return ETy{nullptr, node.type(), false, false};
   } else {
      bool isParentClass = dyn_cast<ast::ClassDecl>(node.decl()->parent());
      bool isStaticModifier = isDeclStatic(node.decl());
      bool isInstanceVar = isParentClass && !isStaticModifier;
      return ETy{node.decl(), node.type(), true, isInstanceVar};
   }
}

ETy ESC::evalBinaryOp(BinaryOp& op, const ETy lhs, const ETy rhs) const {
   assert(op.resultType());
   if((lhs.isInstanceVar || rhs.isInstanceVar) && isStaticContext)
      throw IllegalInstanceAccess();
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalUnaryOp(UnaryOp& op, const ETy val) const {
   assert(op.resultType());
   if(val.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalMemberAccess(DotOp& op, const ETy lhs, const ETy field) const {
   assert(op.resultType());
   // LHS may never be a type (it might not have a decl for ex. temporaries)
   assert(lhs.isValue);
   // RHS must be a field and have a resolved declaration
   assert(field.isValue && field.decl);
   // Only check if LHS is an instance variable
   if(lhs.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   // The field must not be static because this is "instance . field"
   if(isDeclStatic(field.decl)) {
      throw diag.ReportError(loc_)
            << "cannot access a static field through an instance variable";
   }
   // See NOTE in ETy on why instance var is false here!
   return ETy{field.decl, op.resultType(), true, false};
}

ETy ESC::evalMethodCall(MethodOp& op, const ETy method,
                        const op_array& args) const {
   // Method must be value and have a resolved declaration
   assert(method.isValue && method.decl);
   // Accessing instance method in a static context
   if(method.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   // And also all the arguments
   for(auto arg : args) {
      assert(arg.isValue);
      if(arg.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   }
   // ... any other checks ... ?
   // We don't assert op.resultType() because it can be null i.e., void
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalNewObject(NewOp& op, const ETy ty, const op_array& args) const {
   assert(!ty.isValue && ty.type);
   for(auto arg : args) {
      assert(arg.isValue);
      if(arg.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   }
   assert(op.resultType());
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalNewArray(NewArrayOp& op, const ETy type, const ETy size) const {
   assert(!type.isValue && type.type);
   assert(size.isValue);
   if(size.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   assert(op.resultType());
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalArrayAccess(ArrayAccessOp& op, const ETy arr, const ETy idx) const {
   assert(arr.isValue);
   assert(idx.isValue);
   if((arr.isInstanceVar || idx.isInstanceVar) && isStaticContext)
      throw IllegalInstanceAccess();
   assert(op.resultType());
   return ETy{nullptr, op.resultType(), true, false};
}

ETy ESC::evalCast(CastOp& op, const ETy type, const ETy obj) const {
   assert(!type.isValue && type.type);
   assert(obj.isValue);
   assert(op.resultType());
   if(obj.isInstanceVar && isStaticContext) throw IllegalInstanceAccess();
   return ETy{nullptr, op.resultType(), true, false};
}

} // namespace semantic
