
#ifndef TOKEN_H_
#define TOKEN_H_

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

struct TokenInfo {
  int tok;
  std::string identifierStr;
  double numVal;
};

TokenInfo gettok();
#endif

