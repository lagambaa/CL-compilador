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

#pragma once

#include "antlr4-runtime.h"
#include "AslBaseVisitor.h"

#include "../common/TypesMgr.h"
#include "../common/SymTable.h"
#include "../common/TreeDecoration.h"
#include "../common/SemErrors.h"

// using namespace std;


//////////////////////////////////////////////////////////////////////
// Class TypeCheckVisitor: derived from AslBaseVisitor.
// The tree visitor go through the parser tree and call the methods of
// this class to do the semantic typecheck of the program. This is
// done once the SymbolsVisitor has finish and all the symbols of the
// program has been added to their respective scope. In this visit,
// if some node/method does not have an associated task, it does not
// have to be visited/called so no redefinition is needed.

class TypeCheckVisitor final : public AslBaseVisitor {

public:

  // Constructor
  TypeCheckVisitor(TypesMgr       & Types,
                   SymTable       & Symbols,
                   TreeDecoration & Decorations,
                   SemErrors      & Errors);

  // Methods to visit each kind of node.
  // Non visited nodes have been commented out:
  std::any visitProgram(AslParser::ProgramContext *ctx);
  std::any visitFunction(AslParser::FunctionContext *ctx);
  // std::any visitDeclarations(AslParser::DeclarationsContext *ctx);
  // std::any visitVariable_decl(AslParser::Variable_declContext *ctx);
  // std::any visitType(AslParser::TypeContext *ctx);
  std::any visitStatements(AslParser::StatementsContext *ctx);
  std::any visitAssignStmt(AslParser::AssignStmtContext *ctx);
  std::any visitIfStmt(AslParser::IfStmtContext *ctx);
  std::any visitWhileStmt(AslParser::WhileStmtContext *ctx);
  std::any visitProcCall(AslParser::ProcCallContext *ctx);
  std::any visitReadStmt(AslParser::ReadStmtContext *ctx);
  std::any visitWriteExpr(AslParser::WriteExprContext *ctx);
  std::any visitParenthesis(AslParser::ParenthesisContext *ctx);
  std::any visitNegate(AslParser::NegateContext *ctx);
  // std::any visitWriteString(AslParser::WriteStringContext *ctx);
  std::any visitLeft_expr(AslParser::Left_exprContext *ctx);
  std::any visitExprIdent(AslParser::ExprIdentContext *ctx);
  std::any visitArithmetic(AslParser::ArithmeticContext *ctx);
  std::any visitRelational(AslParser::RelationalContext *ctx);
  std::any visitLogical(AslParser::LogicalContext *ctx);
  std::any visitValue(AslParser::ValueContext *ctx);
  std::any visitIdent(AslParser::IdentContext *ctx);
  std::any visitReturnStmt(AslParser::ReturnStmtContext *ctx);

  std::any visitLeftIdent(AslParser::LeftIdentContext *ctx);
  std::any visitLeftArrayAccess(AslParser::LeftArrayAccessContext *ctx);
  std::any visitArrayAccess(AslParser::ArrayAccessContext *ctx);
  std::any visitFunctionCall(AslParser::FunctionCallContext *ctx);
private:

  // Attributes
  TypesMgr       & Types;
  SymTable       & Symbols;
  TreeDecoration & Decorations;
  SemErrors      & Errors;
  // Current function type (assigned before visit its instructions)
  TypesMgr::TypeId currFunctionType;

  // Accessor/Mutator to the type (TypeId) of the current function
  TypesMgr::TypeId getCurrentFunctionTy ()                      const;
  void             setCurrentFunctionTy (TypesMgr::TypeId type);

  // Getters for the necessary tree node atributes:
  //   Scope, Type ans IsLValue
  SymTable::ScopeId getScopeDecor    (antlr4::ParserRuleContext *ctx);
  TypesMgr::TypeId  getTypeDecor     (antlr4::ParserRuleContext *ctx);
  bool              getIsLValueDecor (antlr4::ParserRuleContext *ctx);

  // Setters for the necessary tree node attributes:
  //   Scope, Type ans IsLValue
  void putScopeDecor    (antlr4::ParserRuleContext *ctx, SymTable::ScopeId s);
  void putTypeDecor     (antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t);
  void putIsLValueDecor (antlr4::ParserRuleContext *ctx, bool b);

};  // class TypeCheckVisitor
