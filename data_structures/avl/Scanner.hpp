#include <cstring>
#include <sstream>
#include <stdlib.h>
#include "Token.hpp"
#include <unordered_map>


class Scanner {
public:
  Scanner();
  Scanner(string in_s);
  Scanner(const char* in_s);
  Token* nextToken();
  void resetScanner();
  void setInput(string new_s);
  ~Scanner();  
private:
  string input;
  int first, current;
  unordered_map<string, Token::Type> reserved;
  void init_reserved();
  char nextChar();
  void rollBack();
  void startLexema();
  string getLexema();
  Token::Type checkReserved(string lexema);
};

void Scanner::init_reserved() {
  reserved["create"] = Token::CREATE;
  reserved["delete"] = Token::DELETE;
  reserved["table"] = Token::TABLE;
  reserved["insert"] = Token::INSERT;
  reserved["into"] = Token::INTO;
  reserved["using"] = Token::USING;
  reserved["index"] = Token::INDEX;
  reserved["select"] = Token::SELECT;
  reserved["from"] = Token::FROM;
  reserved["where"] = Token::WHERE;
  reserved["on"] = Token::ON;
  reserved["file"] = Token::FILE;
  reserved["values"] = Token::VALUES;
  reserved["hash"] = Token::HASH;
  reserved["seq"] = Token::SEQ;
  reserved["and"] = Token::AND;
  reserved["or"] = Token::OR;
  reserved["between"] = Token::BETWEEN;
}

Scanner::Scanner(): input(""), first(0), current(0) {
  init_reserved();
}

Scanner::Scanner(string in_s): input(in_s), first(0), current(0) {
  init_reserved();
}

Scanner::Scanner(const char* in_s): input(in_s), first(0), current(0) {
  init_reserved();
}

void Scanner::setInput(string new_s) {
  this->input = new_s;
}

void Scanner::resetScanner() { first = 0; current = 0; }

Token::Type Scanner::checkReserved(string lexema) {
  std::unordered_map<std::string,Token::Type>::const_iterator it = reserved.find(lexema);
  if (it == reserved.end())
    return Token::ERR;
  else
    return it->second;
}


Token* Scanner::nextToken() {
  Token* token;
  char c;
  // consume whitespaces
  c = nextChar();
  while (c == ' ') c = nextChar();
  if (c == '\0') return new Token(Token::END);
  startLexema();

  if (c == '\'') {
    c = nextChar();
    while (c != '\'')  c = nextChar();
    string lex = getLexema();
    token = new Token(Token::STRING, lex.substr(1,lex.size()-2));
  }
  else if (c == '\"') {
    c = nextChar();
    while (c != '\"')  c = nextChar();
    string lex = getLexema();
    token = new Token(Token::STRING, lex.substr(1,lex.size()-2));
  }

  else if (isdigit(c)) {
    c = nextChar();
    while (isdigit(c)) c = nextChar();

    if (c == '.') {
      c = nextChar();
      if (isdigit(c)) {
        while (isdigit(c)) c = nextChar();
        rollBack();
        token = new Token(Token::FLOAT, getLexema());
      } else {
        token = new Token(Token::ERR, c);
      }
    } else {
      rollBack();
      token = new Token(Token::NUM, getLexema());
    }
  } 
  
  else if (isalpha(c) || c == '_') {
    c = nextChar();
    while (isalpha(c) || c == '_')  c = nextChar();
    rollBack();

    string lex = getLexema();
    Token::Type ttype = checkReserved(lex);
    if (ttype != Token::ERR) {
      token = new Token(ttype);
    } else {
      token = new Token(Token::ID, getLexema());
    }
  } 
  
  else if (strchr("*(),;<>=", c)) {
    switch(c) {
      case '(': token = new Token(Token::LPAREN); break;
      case ')': token = new Token(Token::RPAREN); break;
      case '*': token = new Token(Token::ALL); break;
      case ',': token = new Token(Token::COMMA); break;
      case ';': token = new Token(Token::PTCOMMA); break;
      case '=': token = new Token(Token::EQUAL); break;
      case '<':
        c = nextChar();
        if (c == '=')
          token = new Token(Token::LESSEQUAL);
        else {
          rollBack();
          token = new Token(Token::LESS);
        }; 
        break;

      case '>':
        c = nextChar();
        if (c == '=')
          token = new Token(Token::GREATEREQUAL);
        else {
          rollBack();
          token = new Token(Token::GREATER);
        }; 
        break;
    }
  } else {
    token = new Token(Token::ERR, c);
  }
  cout << "next token " << token << endl;
  return token;
}


Scanner::~Scanner() { }

char Scanner::nextChar() {
    int c = input[current];
    current++;
    return c;
}

void Scanner::rollBack() { // retract
    current--;
}

void Scanner::startLexema() {
    first = current-1;
    return;
}

string Scanner::getLexema() {
    return input.substr(first,current-first);
}