#include <iostream>
#include <vector>
#include <memory>

// type definition
enum Token {
  tok_eof = -1,
  tok_def = -2,
  tok_extern = -3,
  tok_identifier = -4,
  tok_number = -5,
  tok_comm = -6,
  tok_nline = -7,
  tok_unknown = -8,
};

class ExprAST {
public: 
  virtual ~ExprAST() {}
};

class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double val) : Val(val) {}
};

class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &name) : Name(name) {}
};

class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs,
                std::unique_ptr<ExprAST> rhs)
    : Op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}
};

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &callee, 
              std::vector<std::unique_ptr<ExprAST>> args)
    : Callee(callee), Args(std::move(args)) {}
};

class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(std::string name, std::vector<std::string> args)
    : Name(name), Args(std::move(args)) {}

  const std::string &getName() const {return Name; }
};

class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> proto,
              std::unique_ptr<ExprAST> body) 
    : Proto(std::move(proto)), Body(std::move(body)) {}
};

// function definition
static void dump_token(Token tok);
static int gettok();
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

