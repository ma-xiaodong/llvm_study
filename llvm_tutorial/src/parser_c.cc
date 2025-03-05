#include <iostream>
#include <map>
#include <string>
#include "../include/token.h"
#include "../include/parser_c.h"

static std::string IdentifierStr;
static double NumVal;
static int CurTok;
static std::map<char, int> BinopPrecedence;
// char corresponding to unknown token
static char ThisChar;

static void dump_token(Token tok);
static int getNextToken();
static std::unique_ptr<ExprAST> LogError(const char *str);
static std::unique_ptr<ExprAST> ParseNumberExpr();
static std::unique_ptr<ExprAST> ParseParenExpr();
static std::unique_ptr<ExprAST> ParseIndentifierExpr();
static std::unique_ptr<ExprAST> ParsePrimary();
static std::unique_ptr<ExprAST> ParseExpression();
static int GetTokPrecedence();
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS);
static std::unique_ptr<PrototypeAST> ParsePrototype();
static std::unique_ptr<FunctionAST> ParseDefinition();
static std::unique_ptr<PrototypeAST> ParseExtern();
static std::unique_ptr<FunctionAST> ParseTopLevelExpr();
static void MainLoop();
static void HandleDefinition();
static void HandleExtern();
static void HandleTopLevelExpression();

// lexer, recoginze tokens
static void dump_token(Token tok) {
  switch (tok) {
    case tok_eof:
      std::cout << "end of file." << std::endl;
      break;
    case tok_def:
      std::cout << "def" << std::endl;
      break;
    case tok_extern:
      std::cout << "extern" << std::endl;
      break;
    case tok_identifier:
      std::cout << "identifier: " << IdentifierStr << std::endl;
      break;
    case tok_number:
      std::cout << NumVal << std::endl;
      break;
    case tok_nline:
      std::cout << "nline: " << IdentifierStr << std::endl;
      break;
    case tok_comm:
      std::cout << "comment" << std::endl;
      break;
    default:
      std::cout << "Unknown token!" << std::endl;
      break;
  }
}

static int getNextToken() {
  TokenInfo tokInfo;

  tokInfo = gettok();
  CurTok = tokInfo.tok;
  IdentifierStr = tokInfo.identifierStr;
  NumVal = tokInfo.numVal;

  return CurTok;
}

// log function
std::unique_ptr<ExprAST> LogError(const char *str) {
  fprintf(stderr, "LogErr: %s\n", str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *str) {
  LogError(str);
  return nullptr;
}

// parser, get the AST
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken();
  return std::move(Result);
}

static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat '('
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ')'
  return V;
}

static std::unique_ptr<ExprAST> ParseIndentifierExpr() {
  std::string IdName = IdentifierStr;

  getNextToken(); // eat identifier

  if (CurTok != '(') // simple variable
    return std::make_unique<VariableExprAST>(IdName);

  // else, it must be a call
  getNextToken(); // eat '('
  std::vector<std::unique_ptr<ExprAST>> args;
  if (CurTok != ')') {
    while (1) {
      if (auto arg = ParseExpression())
	args.push_back(std::move(arg));
      else
	return nullptr;

      if (CurTok == ')')
	break;

      if (CurTok != ',')
	LogError("Expected ')' or ',' in argument");

      getNextToken();
    }
  }

  getNextToken(); // eat the ')'
  return std::make_unique<CallExprAST>(IdName, std::move(args));
}

// wrap the parser into one entry
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  case tok_identifier:
    return ParseIndentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  default:
    return LogError("Unknown token when expecting an expression!");
  }
}

static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  while (1) {
    int TokPrec = GetTokPrecedence();

    if (TokPrec < ExprPrec)
      return LHS;

    int BinOp = CurTok;
    getNextToken();
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      // associate RHS with expr after NextPrec
      RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS));
      if (!RHS)
	return nullptr;
    }
    // combine LHS and RHS as new LHS
    LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS),
                                          std::move(RHS));
  }
}

static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name!");

  std::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')  // eat '('
    return LogErrorP("Expected '(' in prototype!");

  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier) {
    ArgNames.push_back(IdentifierStr);
  }
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype!");

  // sucess
  getNextToken();

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken();  // eat def
  // parse the definition
  auto Proto = ParsePrototype();
  if (!Proto) return nullptr;

  // parse the body
  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // eat extern
  return ParsePrototype();
}

static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

static void MainLoop() {
  while (1) {
    switch (CurTok) {
    case tok_eof:
      return;
    case ';': // ignore top-level semicolons
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

static void HandleDefinition() {
  if (ParseDefinition()) {
    fprintf(stderr, "Parse a function definition.\n");
  } else {
    getNextToken();
  }
}

static void HandleExtern() {
  if (ParseExtern()) {
    fprintf(stderr, "Parse an extern.\n");
  } else {
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  if (ParseTopLevelExpr()) {
    fprintf(stderr, "Parse a top-level expr.\n");
  } else {
    getNextToken();
  }
}

static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0) return -1;
  return TokPrec;
}

int main() {
  // initialize the precedence
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 10;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 10;

  getNextToken();

  MainLoop();
  return 0;
}

