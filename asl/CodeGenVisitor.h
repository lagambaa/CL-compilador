//////////////////////////////////////////////////////////////////////
//
//    CodeGenVisitor - Walk the parser tree to do
//                     the generation of code
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
#include "../common/code.h"

#include <string>

// using namespace std;


//////////////////////////////////////////////////////////////////////
// Class CodeGenVisitor: derived from AslBaseVisitor.
// The tree visitor go through the parser tree and call the methods of
// this class to generate the code of the program. This is done
// once the SymbolsVisitor and TypeCheckVisitor have finish with no
// semantic error. So all the symbols of the program has been added to
// their respective scope and the type of each expresion has also be
// computed and decorate the parse tree. In this visit, if some node/method
// does not have an associated task, it does not have to be visited/called
// so no redefinition is needed.

class CodeGenVisitor final : public AslBaseVisitor {

public:

  // Constructor
  CodeGenVisitor(TypesMgr       & Types,
                 SymTable       & Symbols,
                 TreeDecoration & Decorations);

  // Methods to visit each kind of node:
  std::any visitProgram(AslParser::ProgramContext *ctx);
  std::any visitFunction(AslParser::FunctionContext *ctx);
  std::any visitDeclarations(AslParser::DeclarationsContext *ctx);
  std::any visitVariable_decl(AslParser::Variable_declContext *ctx);
  // std::any visitType(AslParser::TypeContext *ctx);
  std::any visitStatements(AslParser::StatementsContext *ctx);
  std::any visitAssignStmt(AslParser::AssignStmtContext *ctx);
  std::any visitIfStmt(AslParser::IfStmtContext *ctx);
  std::any visitProcCall(AslParser::ProcCallContext *ctx);
  std::any visitReadStmt(AslParser::ReadStmtContext *ctx);
  std::any visitWriteExpr(AslParser::WriteExprContext *ctx);
  std::any visitWriteString(AslParser::WriteStringContext *ctx);
  std::any visitLeft_expr(AslParser::Left_exprContext *ctx);
  std::any visitExprIdent(AslParser::ExprIdentContext *ctx);
  std::any visitArithmetic(AslParser::ArithmeticContext *ctx);
  std::any visitRelational(AslParser::RelationalContext *ctx);
  std::any visitValue(AslParser::ValueContext *ctx);
  std::any visitIdent(AslParser::IdentContext *ctx);

  std::any visitLeftIdent(AslParser::LeftIdentContext *ctx);
  std::any visitLeftArrayAccess(AslParser::LeftArrayAccessContext *ctx);
  std::any visitArrayAccess(AslParser::ArrayAccessContext *ctx);

private:

  // Attributes
  TypesMgr        & Types;
  SymTable        & Symbols;
  TreeDecoration  & Decorations;
  counters          codeCounters;
  // Current function type (assigned before visit its instructions)
  TypesMgr::TypeId currFunctionType;

  // Accessor/Mutator to the type (TypeId) of the current function
  TypesMgr::TypeId getCurrentFunctionTy ()                      const;
  void             setCurrentFunctionTy (TypesMgr::TypeId type);

  // Getters for the necessary tree node atributes:
  //   Scope and Type
  SymTable::ScopeId getScopeDecor (antlr4::ParserRuleContext *ctx) const;
  TypesMgr::TypeId  getTypeDecor  (antlr4::ParserRuleContext *ctx) const;


  //////////////////////////////////////////////////////////////////
  // Class CodeAttribs: is declared inside CodeGenVisitor as an
  // auxiliary class to group the three attributes necessaries for
  // code generation (address, offset, instructions list).
  // Some language constructions, for example expressions, can
  // generate the three attributes. Others, like statements, only
  // generate the instruction list.
  class CodeAttribs {
    
  public:
    // Constructors
    CodeAttribs(const std::string & addr,
                const std::string & offs,
                instructionList & code);
    CodeAttribs(const std::string & addr,
                const std::string & offs,
                instructionList && code);

    // Attributes (publics):
    //   - the address that will hold the value of an expression
    std::string addr;
    //   - the offset applied to the address (for array access)
    std::string offs;
    //   - the three-address code associated to an statement/expression
    instructionList code;

  };  // class CodeAttribs
  
};  // class CodeGenVisitor
