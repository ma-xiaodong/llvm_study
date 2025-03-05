#include <iostream>
#include <map>
#include <string>
#include "../include/token.h"

static char LastChar = ' ', ThisChar;
TokenInfo gettok() {
  TokenInfo retValue;
  std::string IdentifierStr;
  double NumVal;

  while (isspace(LastChar))
    LastChar = getchar();

  // identifier or keywords (def, extern)
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getchar())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def") {
      retValue.tok = tok_def;
      return retValue;
    }
    if (IdentifierStr == "extern") {
      retValue.tok = tok_extern;
      return retValue;
    }
    retValue.tok = tok_identifier;
    retValue.identifierStr = IdentifierStr;
    return retValue;
  }

  // numbers
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');
    NumVal = strtod(NumStr.c_str(), 0);

    retValue.tok = tok_number;
    retValue.numVal = NumVal;
    return retValue;
  }

  // eof token
  if (LastChar == EOF) {
    retValue.tok = tok_eof;
    return retValue;
  }

  // new line
  if (LastChar == '\n' || LastChar == '\r') {
    retValue.tok = tok_nline;
    return retValue;
  }

  // comments
  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    retValue.tok = tok_comm;
    return retValue;
  }

  ThisChar = LastChar;
  LastChar = getchar();
  // std::cout << "WARNNING: unknown char : " << ThisChar << std::endl;
  retValue.tok = ThisChar;
  return retValue;
}

