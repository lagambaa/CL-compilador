//////////////////////////////////////////////////////////////////////
//
//    TypeCheckVisitor - Walk the parser tree to do the semantic
//                       typecheck for the Asl programming language
//
//    Copyright (C) 2020-2030  Universitat Politecnica de Catalunya
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 3
//    of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    contact: José Miguel Rivero (rivero@cs.upc.edu)
//             Computer Science Department
//             Universitat Politecnica de Catalunya
//             despatx Omega.110 - Campus Nord UPC
//             08034 Barcelona.  SPAIN
//
//////////////////////////////////////////////////////////////////////

#include "TypeCheckVisitor.h"
#include "antlr4-runtime.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

#include <iostream>
#include <string>

// uncomment the following line to enable debugging messages with DEBUG*
// #define DEBUG_BUILD
#include "../common/debug.h"

// using namespace std;


// Constructor
TypeCheckVisitor::TypeCheckVisitor(TypesMgr       & Types,
                                   SymTable       & Symbols,
                                   TreeDecoration & Decorations,
                                   SemErrors      & Errors) :
  Types{Types},
  Symbols{Symbols},
  Decorations{Decorations},
  Errors{Errors} {
}

// Accessor/Mutator to the attribute currFunctionType
TypesMgr::TypeId TypeCheckVisitor::getCurrentFunctionTy() const {
  return currFunctionType;
}

void TypeCheckVisitor::setCurrentFunctionTy(TypesMgr::TypeId type) {
  currFunctionType = type;
}

// Methods to visit each kind of node:
//
std::any TypeCheckVisitor::visitProgram(AslParser::ProgramContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);
  for (auto ctxFunc : ctx->function()) { 
    visit(ctxFunc);
  }
  if (Symbols.noMainProperlyDeclared())
    Errors.noMainProperlyDeclared(ctx);
  Symbols.popScope();
  Errors.print();
  DEBUG_EXIT();
  return 0;
}

// std::any TypeCheckVisitor::visitFunction(AslParser::FunctionContext *ctx) {
//   DEBUG_ENTER();
//   SymTable::ScopeId sc = getScopeDecor(ctx);
//   Symbols.pushThisScope(sc);

//   std::string fname = ctx->ID()->getText();
//   TypesMgr::TypeId tFunc = Symbols.getType(fname);
//   setCurrentFunctionTy(tFunc); 

//   // Symbols.print();
//   visit(ctx->statements());
//   Symbols.popScope();
//   DEBUG_EXIT();
//   return 0;
// }

std::any TypeCheckVisitor::visitFunction(AslParser::FunctionContext *ctx) {
  DEBUG_ENTER();
  SymTable::ScopeId sc = getScopeDecor(ctx);
  Symbols.pushThisScope(sc);

  TypesMgr::TypeId tFunc;
  if (ctx->basic_type()) tFunc = getTypeDecor(ctx->basic_type());
  else tFunc = Types.createVoidTy();

  setCurrentFunctionTy(tFunc); 

  // Symbols.print();
  visit(ctx->statements());
  Symbols.popScope();
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitReturnStmt(AslParser::ReturnStmtContext *ctx) {
  DEBUG_ENTER();
  
  TypesMgr::TypeId tRet = getCurrentFunctionTy();
  if (ctx->expr()) {
    visit(ctx->expr());
    TypesMgr::TypeId tExpr = getTypeDecor(ctx->expr());
    if ((not Types.isErrorTy(tExpr)) and
        (not Types.copyableTypes(tRet, tExpr))) {
      Errors.incompatibleReturn(ctx->RET());
    }
  }
  else {
    //if (not Types.isVoidTy(tRet)) {
    if (not Types.isVoidTy(tRet)) {
      Errors.incompatibleReturn(ctx->RET());
    }
  }

  DEBUG_EXIT();
  return 0;
}

// std::any TypeCheckVisitor::visitDeclarations(AslParser::DeclarationsContext *ctx) {
//   DEBUG_ENTER();
//   std::any r = visitChildren(ctx);
//   DEBUG_EXIT();
//   return r;
// }

// std::any TypeCheckVisitor::visitVariable_decl(AslParser::Variable_declContext *ctx) {
//   DEBUG_ENTER();
//   std::any r = visitChildren(ctx);
//   DEBUG_EXIT();
//   return r;
// }

// std::any TypeCheckVisitor::visitType(AslParser::TypeContext *ctx) {
//   DEBUG_ENTER();
//   std::any r = visitChildren(ctx);
//   DEBUG_EXIT();
//   return r;
// }

std::any TypeCheckVisitor::visitStatements(AslParser::StatementsContext *ctx) {
  DEBUG_ENTER();
  visitChildren(ctx);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitAssignStmt(AslParser::AssignStmtContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->left_expr());
  visit(ctx->expr());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.copyableTypes(t1, t2)))
    Errors.incompatibleAssignment(ctx->ASSIGN());
  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableLeftExpr(ctx->left_expr());
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitIfStmt(AslParser::IfStmtContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1))) {
    Errors.booleanRequired(ctx);
  }

  visit(ctx->statements(0));
  if (ctx->ELSE()) visit(ctx->statements(1));   

  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitWhileStmt(AslParser::WhileStmtContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isBooleanTy(t1)))
    Errors.booleanRequired(ctx);
  visit(ctx->statements());
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitProcCall(AslParser::ProcCallContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->ident());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  if (not Types.isErrorTy(t1) and not Types.isFunctionTy(t1)) {
    Errors.isNotCallable(ctx->ident());
  }

  if (ctx->args()) visit(ctx->args());

  if (not Types.isErrorTy(t1) and Types.isFunctionTy(t1)) {
    size_t numArgs = ctx->args() ? ctx->args()->expr().size() : 0;
    size_t numParams = Types.getNumOfParameters(t1);
    if (numArgs != numParams) {
      Errors.numberOfParameters(ctx->ident());
    }
    else {
      for (size_t i = 0; i < numArgs; ++i) {
        TypesMgr::TypeId tArg = getTypeDecor(ctx->args()->expr(i));
        TypesMgr::TypeId tParam = Types.getParameterType(t1, i);
        if (not Types.isErrorTy(tArg) and not Types.copyableTypes(tParam, tArg)) {
          Errors.incompatibleParameter(ctx->args()->expr(i), i + 1, ctx);
        }
      }
    }
  }

  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitFunctionCall(AslParser::FunctionCallContext *ctx) {
    DEBUG_ENTER();
    visit(ctx->ident());
    TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
    if (not Types.isErrorTy(t1) and not Types.isFunctionTy(t1)) {
      Errors.isNotCallable(ctx->ident());
    }

    if (ctx->args()) visit(ctx->args());

    if (not Types.isErrorTy(t1) and Types.isFunctionTy(t1)) {
      size_t numArgs = ctx->args() ? ctx->args()->expr().size() : 0;
      size_t numParams = Types.getNumOfParameters(t1);
      if (numArgs != numParams) {
        Errors.numberOfParameters(ctx->ident());
      }
      else {
        for (size_t i = 0; i < numArgs; ++i) {
          TypesMgr::TypeId tArg = getTypeDecor(ctx->args()->expr(i));
          TypesMgr::TypeId tParam = Types.getParameterType(t1, i);
          if (not Types.isErrorTy(tArg) and not Types.copyableTypes(tParam, tArg)) {
            Errors.incompatibleParameter(ctx->args()->expr(i), i + 1, ctx);
          }
        }
      }
    }

    TypesMgr::TypeId t;
    if (not Types.isErrorTy(t1) && Types.isFunctionTy(t1) && Types.isVoidFunction(t1)) {
      Errors.isNotFunction(ctx->ident());
      t = Types.createErrorTy();
    }
    else {
      t = (Types.isFunctionTy(t1)) ? Types.getFuncReturnType(t1) : Types.createErrorTy();
    }

    putTypeDecor(ctx, t);
    putIsLValueDecor(ctx, false);
    DEBUG_EXIT();
    return 0;
}

std::any TypeCheckVisitor::visitReadStmt(AslParser::ReadStmtContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->left_expr());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->left_expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)) and
      (not Types.isFunctionTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  if ((not Types.isErrorTy(t1)) and (not getIsLValueDecor(ctx->left_expr())))
    Errors.nonReferenceableExpression(ctx);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitWriteExpr(AslParser::WriteExprContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isPrimitiveTy(t1)))
    Errors.readWriteRequireBasic(ctx);
  DEBUG_EXIT();
  return 0;
}

// std::any TypeCheckVisitor::visitWriteString(AslParser::WriteStringContext *ctx) {
//   DEBUG_ENTER();
//   std::any r = visitChildren(ctx);
//   DEBUG_EXIT();
//   return r;
// }

// std::any TypeCheckVisitor::visitLeft_expr(AslParser::Left_exprContext *ctx) {
//   DEBUG_ENTER();
//   visit(ctx->ident());
//   TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
//   putTypeDecor(ctx, t1);
//   bool b = getIsLValueDecor(ctx->ident());
//   putIsLValueDecor(ctx, b);
//   DEBUG_EXIT();
//   return 0;
// }

std::any TypeCheckVisitor::visitLeftIdent(AslParser::LeftIdentContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->ident());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  putTypeDecor(ctx, t1);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
  return 0;
}


std::any TypeCheckVisitor::visitLeftArrayAccess(AslParser::LeftArrayAccessContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->ident());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  visit(ctx->expr());
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isArrayTy(t1)))
    Errors.nonArrayInArrayAccess(ctx);
  if ((not Types.isErrorTy(t2)) and (not Types.isIntegerTy(t2)))
    Errors.nonIntegerIndexInArrayAccess(ctx->expr());
  
  TypesMgr::TypeId t = (Types.isArrayTy(t1)) ? Types.getArrayElemType(t1) : Types.createErrorTy();
  putTypeDecor(ctx, t);
  //bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, true);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitArrayAccess(AslParser::ArrayAccessContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->ident());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  visit(ctx->expr());
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr());
  if ((not Types.isErrorTy(t1)) and (not Types.isArrayTy(t1)))
    Errors.nonArrayInArrayAccess(ctx);
  if ((not Types.isErrorTy(t2)) and (not Types.isIntegerTy(t2)))
    Errors.nonIntegerIndexInArrayAccess(ctx->expr());
  
  TypesMgr::TypeId t = (Types.isArrayTy(t1)) ? Types.getArrayElemType(t1) : Types.createErrorTy();
  putTypeDecor(ctx, t);
  //bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitArithmetic(AslParser::ArithmeticContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr(0));
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  visit(ctx->expr(1));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  if (ctx->MOD()) {
    if (
      (not Types.isErrorTy(t1) and not Types.isIntegerTy(t1)) or
      (not Types.isErrorTy(t2) and not Types.isIntegerTy(t2))
    )
    Errors.incompatibleOperator(ctx->op);
  }
  else {
    if (((not Types.isErrorTy(t1)) and (not Types.isNumericTy(t1))) or
        ((not Types.isErrorTy(t2)) and (not Types.isNumericTy(t2)))) 
      Errors.incompatibleOperator(ctx->op);
  }

  TypesMgr::TypeId t;
  if (Types.isErrorTy(t1) or Types.isErrorTy(t2)) {
    t = Types.createErrorTy();
  }
  else if (ctx->MOD()) {
    t = Types.createIntegerTy();
  }
  else if (Types.isFloatTy(t1) or Types.isFloatTy(t2)) {
    t = Types.createFloatTy();
  }
  else {
    t = Types.createIntegerTy();
  }
  
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitRelational(AslParser::RelationalContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr(0));
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  visit(ctx->expr(1));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  std::string oper = ctx->op->getText();
  if ((not Types.isErrorTy(t1)) and (not Types.isErrorTy(t2)) and
      (not Types.comparableTypes(t1, t2, oper)))
    Errors.incompatibleOperator(ctx->op);

  TypesMgr::TypeId t = Types.createBooleanTy();
  // sembla correcte pero SEMPRE retornen bool per tant no s'ha de retornar errortype com a arithmetic
  // if (Types.isErrorTy(t1) or Types.isErrorTy(t2)) t = Types.createErrorTy();
  // else t = Types.createBooleanTy();
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitLogical(AslParser::LogicalContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr(0));
  TypesMgr::TypeId t1 = getTypeDecor(ctx->expr(0));
  visit(ctx->expr(1));
  TypesMgr::TypeId t2 = getTypeDecor(ctx->expr(1));
  // std::string oper = ctx->op->getText();
  if (
    (not Types.isErrorTy(t1) and not Types.isBooleanTy(t1))
    or (not Types.isErrorTy(t2) and not Types.isBooleanTy(t2))) {
    Errors.incompatibleOperator(ctx->op);
  }
  TypesMgr::TypeId t = Types.createBooleanTy();;
  // sembla correcte pero SEMPRE retornen bool per tant no s'ha de retornar errortype com a arithmetic
  // if (Types.isErrorTy(t1) or Types.isErrorTy(t2)) t = Types.createErrorTy();
  // else t = Types.createBooleanTy();
  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitNegate(AslParser::NegateContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr());
  TypesMgr::TypeId t = getTypeDecor(ctx->expr());

  // crec que no fa faltareassignar t
  if (ctx->NOT()) {
    if (not Types.isErrorTy(t) and not Types.isBooleanTy(t)) {
      Errors.incompatibleOperator(ctx->op);
    }
     // sembla correcte pero SEMPRE retornA bool per tant no s'ha de retornar errortype com a arithmetic
    // if (Types.isErrorTy(t)) t = Types.createErrorTy();
    // else t = Types.createBooleanTy();
    t = Types.createBooleanTy();
  }
  else {
    if (not Types.isErrorTy(t) and not Types.isNumericTy(t)) {
      Errors.incompatibleOperator(ctx->op);
    }
    
    // aqui si que si que no sempre retorna el mateix, per tant, si errTy el propaguem pq no podem asumir type
    if (Types.isErrorTy(t)) t = Types.createErrorTy();
    else if (Types.isFloatTy(t)) t = Types.createFloatTy();
    else t = Types.createIntegerTy();
  }

  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitParenthesis(AslParser::ParenthesisContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->expr());
  TypesMgr::TypeId t = getTypeDecor(ctx->expr());
  putTypeDecor(ctx, t);
  // (variable) = ... es correcte, per tant el parentesi hade tenir el mateix
  // l-value que la expressio de dins
  bool b = getIsLValueDecor(ctx->expr());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitValue(AslParser::ValueContext *ctx) {
  DEBUG_ENTER();
  TypesMgr::TypeId t;

  auto vexprctx = ctx->valueExpr();
  if (vexprctx->INTVAL()) {
    t = Types.createIntegerTy();
  }
  else if (vexprctx->FLOATVAL()) {
    t = Types.createFloatTy();
  }
  else if (vexprctx->BOOLVAL()) {
    t = Types.createBooleanTy();
  }
  else if (vexprctx->CHARVAL()) {
    t = Types.createCharacterTy();
  }

  putTypeDecor(ctx, t);
  putIsLValueDecor(ctx, false);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitExprIdent(AslParser::ExprIdentContext *ctx) {
  DEBUG_ENTER();
  visit(ctx->ident());
  TypesMgr::TypeId t1 = getTypeDecor(ctx->ident());
  putTypeDecor(ctx, t1);
  bool b = getIsLValueDecor(ctx->ident());
  putIsLValueDecor(ctx, b);
  DEBUG_EXIT();
  return 0;
}

std::any TypeCheckVisitor::visitIdent(AslParser::IdentContext *ctx) {
  DEBUG_ENTER();
  std::string ident = ctx->getText();
  if (Symbols.findInStack(ident) == -1) {
    Errors.undeclaredIdent(ctx->ID());
    TypesMgr::TypeId te = Types.createErrorTy();
    putTypeDecor(ctx, te);
    putIsLValueDecor(ctx, true);
  }
  else {
    TypesMgr::TypeId t1 = Symbols.getType(ident);
    putTypeDecor(ctx, t1);
    if (Symbols.isFunctionClass(ident))
      putIsLValueDecor(ctx, false);
    else
      putIsLValueDecor(ctx, true);
  }
  DEBUG_EXIT();
  return 0;
}


// Getters for the necessary tree node atributes:
//   Scope, Type ans IsLValue
SymTable::ScopeId TypeCheckVisitor::getScopeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getScope(ctx);
}
TypesMgr::TypeId TypeCheckVisitor::getTypeDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getType(ctx);
}
bool TypeCheckVisitor::getIsLValueDecor(antlr4::ParserRuleContext *ctx) {
  return Decorations.getIsLValue(ctx);
}

// Setters for the necessary tree node attributes:
//   Scope, Type ans IsLValue
void TypeCheckVisitor::putScopeDecor(antlr4::ParserRuleContext *ctx, SymTable::ScopeId s) {
  Decorations.putScope(ctx, s);
}
void TypeCheckVisitor::putTypeDecor(antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t) {
  Decorations.putType(ctx, t);
}
void TypeCheckVisitor::putIsLValueDecor(antlr4::ParserRuleContext *ctx, bool b) {
  Decorations.putIsLValue(ctx, b);
}
