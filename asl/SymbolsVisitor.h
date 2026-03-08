//////////////////////////////////////////////////////////////////////
//
//    SymbolsVisitor - Walk the parser tree to register symbols
//                     for the Asl programming language
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
// Class SymbolVisitor:  derived from AslBaseVisitor.
// The tree visitor go through the parse tree and call the methods of
// this class to register the symbols of the program in the symbol
// table. In this visit, if some node/method does not have an
// associated task, it does not have to be visited/called so
// no redefinition is needed.

class SymbolsVisitor final : public AslBaseVisitor {

public:

  // Constructor
  SymbolsVisitor(TypesMgr       & Types,
                 SymTable       & Symbols,
                 TreeDecoration & Decorations,
                 SemErrors      & Errors);

  // Methods to visit each kind of node.
  // Non visited nodes have been commented out:
  std::any visitProgram(AslParser::ProgramContext *ctx);
  std::any visitFunction(AslParser::FunctionContext *ctx);
  std::any visitDeclarations(AslParser::DeclarationsContext *ctx);
  std::any visitVariable_decl(AslParser::Variable_declContext *ctx);
  std::any visitType(AslParser::TypeContext *ctx);
  std::any visitBasic_type(AslParser::Basic_typeContext *ctx);
  // std::any visitStatements(AslParser::StatementsContext *ctx);
  // std::any visitAssignStmt(AslParser::AssignStmtContext *ctx);
  // std::any visitIfStmt(AslParser::IfStmtContext *ctx);
  // std::any visitProcCall(AslParser::ProcCallContext *ctx);
  // std::any visitReadStmt(AslParser::ReadStmtContext *ctx);
  // std::any visitWriteExpr(AslParser::WriteExprContext *ctx);
  // std::any visitWriteString(AslParser::WriteStringContext *ctx);
  // std::any visitLeft_expr(AslParser::Left_exprContext *ctx);
  // std::any visitExprIdent(AslParser::ExprIdentContext *ctx);
  // std::any visitArithmetic(AslParser::ArithmeticContext *ctx);
  // std::any visitRelational(AslParser::RelationalContext *ctx);
  // std::any visitValue(AslParser::ValueContext *ctx);
  // std::any visitIdent(AslParser::IdentContext *ctx);

  std::any visitFunctionCall(AslParser::FunctionCallContext *ctx);

private:

  // Attributes:
  TypesMgr       & Types;
  SymTable       & Symbols;
  TreeDecoration & Decorations;
  SemErrors      & Errors;

  // Getters for the necessary tree node atributes:
  //   Scope and Type
  SymTable::ScopeId getScopeDecor (antlr4::ParserRuleContext *ctx);
  TypesMgr::TypeId  getTypeDecor  (antlr4::ParserRuleContext *ctx);

  // Setters for the necessary tree node attributes:
  //   Scope and Type
  void putScopeDecor (antlr4::ParserRuleContext *ctx, SymTable::ScopeId s);
  void putTypeDecor  (antlr4::ParserRuleContext *ctx, TypesMgr::TypeId t);

};  // class SymbolsVisitor
