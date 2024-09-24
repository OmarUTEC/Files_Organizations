#include <string>
#include <iostream>

using namespace std;

class Token {
public:
    enum Type { LPAREN=0, RPAREN, 
                CREATE, TABLE, INSERT, INTO, USING, INDEX, DELETE, ON,
                SELECT, FROM, WHERE,
                FILE, VALUES,
                HASH, ISAM, SEQ,
                AND, OR, BETWEEN,
                LESS, GREATER, LESSEQUAL, GREATEREQUAL, EQUAL,
                COMMA, PTCOMMA,
                ALL, 
                ID, NUM, FLOAT, STRING,
                ERR, END };
    static const char* token_names[35];
    Type type;
    string lexema;
    Token(Type);
    Token(Type, char c);
    Token(Type, const string source);
};

const char* Token::token_names[35] = 
              { "LPAREN", "RPAREN", 
                "CREATE", "TABLE", "INSERT", "INTO", "USING", "INDEX", "DELETE", "ON",
                "SELECT", "FROM", "WHERE", 
                "FILE", "VALUES",
                "HASH", "ISAM", "SEQ",
                "AND", "OR", "BETWEEN",
                "LESS", "GREATER", "LESSEQUAL", "GREATEREQUAL", "EQUAL",
                "COMMA", "PTCOMMA",
                "ALL", 
                "ID", "NUM", "FLOAT", "STRING",
                "ERR", "END" };

Token::Token(Type type):type(type) { lexema = ""; }

Token::Token(Type type, char c):type(type) { lexema = c; }

Token::Token(Type type, const string source):type(type) {
  lexema = source;
}

ostream& operator<<(ostream& outs, const Token & tok )
{
  if (tok.lexema.empty())
    return outs << Token::token_names[tok.type];
  else
    return outs << Token::token_names[tok.type] << "(" << tok.lexema << ")";
}

ostream& operator<<(ostream& outs, const Token* tok ) {
  return outs << *tok;
}