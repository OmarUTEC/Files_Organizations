#include "Scanner.hpp"

#include <vector>
#include <iostream> 
class Parser {
private:
    Scanner* scanner;
    Token *current, *previous;
    string report, reportFin;
    unordered_map<string, vector<string>> memoria;
    unordered_map<string, Token::Type> indexesCoach;
    unordered_map<string, Token::Type> indexesOlympics;

    bool match(Token::Type ttype);
    bool check(Token::Type ttype);
    void advance();
    bool isAtEnd();
    void resetParser();
    void clearMemory();
    void throwParser(string err);

    bool parseSentence();
    bool parseValue();
    bool parseValues();
    
    bool parseCreateSent();
    bool parseInsertSent();
    bool parseSelectSent();
    bool parseIndexSent();
    bool parseDeleteSent();

    int parseAtributes();
    bool parseAtribute();

    bool parseTable();
    bool parseBoolOp();

public:
    Parser(Scanner* scanner);
    void parse();
    string reportParse();
    void setInput(string input);

};

void Parser::setInput(string value) {
    scanner->setInput(value);
}

void Parser::clearMemory() {
    memoria["atributes"].clear();
    memoria["values"].clear();
    memoria["table"].clear();
}

void Parser::throwParser(string err) {
    if (report.empty()) report = err;
    clearMemory();
}

string Parser::reportParse() { return reportFin; }

void Parser::resetParser() {
    scanner->resetScanner();
    if (current)  delete current;
    if (previous) delete previous;
    current = previous = NULL;
    reportFin = report;
    report = "";
}

bool Parser::match(Token::Type ttype) {
    if (check(ttype)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Token::Type ttype) {
    return current->type == ttype;
}

void Parser::advance() {
    if (!isAtEnd()) {
        Token* temp =current;
        if (previous) delete previous;
        current = scanner->nextToken();
        previous = temp;
        if (check(Token::ERR)) {
            cout << "Parse error, unrecognised character: " << current->lexema << endl;
            exit(0);
        }
    }
}

bool Parser::isAtEnd() {
    return (current->type == Token::END);
}

Parser::Parser(Scanner* sc):scanner(sc) {
    previous = current = NULL;
    memoria["atributes"] = vector<string>();
    memoria["values"] = vector<string>();
    memoria["table"] = vector<string>();
    report = "";
    return;
}

bool Parser::parseValue() {
    if (match(Token::STRING) || match(Token::NUM)) {
        memoria["values"].push_back(previous->lexema);
        return true;
    }
    throwParser("Parser error - Se espera un valor");
    return false;
}

bool Parser::parseTable() {
    if (match(Token::ID)) {
        memoria["table"].push_back(previous->lexema);
        return true;
    }
    throwParser("Parser error - Se espera un tabla");
    return false;
}

bool Parser::parseValues() {
    if (!parseValue()) return false;
    while (match(Token::COMMA))
    
        if (!parseValue()) return false;

    return true;
}

bool Parser::parseAtribute() {
    if (match(Token::ID)) {
        memoria["atributes"].push_back(previous->lexema);
    cout << memoria["atributes"].back() << endl;
        return true;
    }
    throwParser("Parser error - Se espera un campo");
    return false;
}

int Parser::parseAtributes() {
    if (match(Token::ALL))
        return 2;
    else if (parseAtribute()) {
        while (match(Token::COMMA)) {
            if (!parseAtribute()) return 0;
        }   
        return 1;
    } 
    return 0;
}

bool Parser::parseInsertSent() {
    bool check;
    if (!match(Token::INTO)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!parseTable()) return false;
    if (!match(Token::VALUES)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (match(Token::LPAREN)) {
        if (!parseValues()) return false;
        if (!match(Token::RPAREN)) {
            throwParser("Parser error - se espera cerra parentesis");
            return false;
        }
        vector<string> values = memoria["values"];

        memoria["values"].clear();
        memoria["table"].clear();
        //if (!execute) {
            //report = "Ejecucion no completada";
            //return false;
        //}
        return true;
    }
    throwParser("Parser error - se espera valores");
    return false;
}

bool Parser::parseDeleteSent() {
    if (!match(Token::FROM)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!parseTable()) return false;
    if (match(Token::WHERE)) {
        if (!parseAtribute()) return false;
        if (match(Token::EQUAL)) {
            if (!parseValue()) return false;
            string table_name = memoria["table"][0];
            string atribute = memoria["atributes"].back();
            string value = memoria["values"].back();
            
            Token::Type pre_index;
            if (memoria["table"][0] == "player")
                pre_index = indexesCoach[atribute];
            else if (memoria["table"][0] == "game")
                pre_index = indexesOlympics[atribute];
            else {
                report = "No existe la tabla con ese nombre";
                memoria["table"].clear();
                memoria["atributes"].clear();
                return false;
            }
            clearMemory();
            //if (!execute) {
            //    report = "Ejecucion no completada";
            //    return false;
            //}
            return true;
        } 
        throwParser("Parser error de sintaxis");
        return false;
    }
    throwParser("Parser error - se espera WHERE");
    return false;
}

bool Parser::parseCreateSent() {
    if (!parseTable()) return false;
    if (!match(Token::FROM)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!match(Token::FILE)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!match(Token::STRING)) {
        throwParser("Parser error - se espera un directorio");
        return false;
    }
    string table_name = memoria["table"][0];
    string file = previous->lexema;

    clearMemory();
    //if (!execute) {
     //   report = "Ejecucion no completada";
      //  return false;
    //}
    // libera la memoria para la sgte query
    memoria["table"].clear();
    return true;
}

bool Parser::parseIndexSent() {
    if (!match(Token::ID)) {
        throwParser("Parser error - Se espera un identificador");
        return false;
    }
    if (!match(Token::ON)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!parseTable()) return false;
    if (!match(Token::USING)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (match(Token::HASH) || match(Token::ISAM) || match(Token::SEQ)) {
        Token::Type token = previous->type;
        if (match(Token::LPAREN)) {
            if (!parseAtribute()) return false;
            if (!match(Token::RPAREN)) {
                throwParser("Parser error - se espera cerra parentesis");
                return false;
            }
            string atribute = memoria["atributes"].back();
            if (memoria["table"][0] == "player")
                indexesCoach[atribute] = token;
            else if (memoria["table"][0] == "game")
                indexesOlympics[atribute] = token;
            else {
                report = "No existe la tabla con ese nombre";
                memoria["table"].clear();
                memoria["atributes"].clear();
                return false;
            }

            memoria["table"].clear();
            memoria["atributes"].clear();
            return true;
        }
    }
    throwParser("Parser error de sintaxis");
    return false;
}


bool Parser::parseSelectSent() {
    int check_atr = parseAtributes();
    if (!check_atr) return false;
    if (!match(Token::FROM)) {
        throwParser("Parser error de sintaxis");
        return false;
    }
    if (!parseTable()) return false;

    if (match(Token::WHERE)) {
        if (!parseAtribute()) return false;
        if (match(Token::LESS) || match(Token::LESSEQUAL) || match(Token::GREATER) || match(Token::GREATEREQUAL) || match(Token::EQUAL)) {
            Token::Type comparator = previous->type;
            if (!parseValue()) return false;

            // consulta select
            string value = memoria["values"][0];
            string k_atrib = memoria["atributes"].back();
            memoria["atributes"].pop_back();
            Token::Type atr_index; 

            if (memoria["table"][0] == "player")
                atr_index = indexesCoach[k_atrib];
            else if (memoria["table"][0] == "game")
                atr_index = indexesOlympics[k_atrib];
            else {
                report = "No existe la tabla con ese nombre";
                clearMemory();
                return false;
            }

            bool execute;
            //if (check_atr == 2) 
             //   execute = select_query(memoria["table"][0], true, {}, k_atrib, value, "", atr_index, comparator);
            //else {
            //    vector<string> atributes = memoria["atributes"];
               // execute = select_query(memoria["table"][0], false, atributes, k_atrib, value, "", atr_index, comparator);
            //}   

            clearMemory();
            if (!execute) {
                report = "Ejecucion no completada";
                return false;
            }
            return true;
        } 
        else if (match(Token::BETWEEN)) {
            if (parseValue()) {
                if (match(Token::AND)) {
                    if (!parseValue()) return false;

                    // ejecuta el select
                    string k_atrib = memoria["atributes"].back();
                    memoria["atributes"].pop_back();
                    string begin = memoria["values"][0];
                    string end = memoria["values"][1];
                    Token::Type atr_index; 

                    if (memoria["table"][0] == "player")
                        atr_index = indexesCoach[k_atrib];
                    else if (memoria["table"][0] == "game")
                        atr_index = indexesOlympics[k_atrib];
                    else {
                        report = "No existe la tabla con ese nombre";
                        clearMemory();
                        return false;
                    }

                    bool execute;
                    if (check_atr == 2) 
                    clearMemory();
                    if (!execute) {
                        report = "Ejecucion no completada";
                        return false;
                    }
                    return true;
                } else {
                    throwParser("Parser error - Se espera un comparador");
                    return false;
                }
            } 
            return false;
        }
        throwParser("Parser error - Se espera un comparador");
        return false;
    }

    bool execute;

    return true;
}


bool Parser::parseSentence() {
    bool res;
    if (match(Token::CREATE)) {
        if (match(Token::TABLE)) {
            res = parseCreateSent();
        } else if (match(Token::INDEX)) {
            res = parseIndexSent();
        } else {
            throwParser("Parser error de sintaxis");
            return false;
        }
    }
    else if (match(Token::INSERT))
        res = parseInsertSent();
    else if (match(Token::DELETE))
        res = parseDeleteSent();
    else if (match(Token::SELECT))
        res = parseSelectSent();
    else {
        throwParser("Parser error - declaracion desconocida");
        return false;
    }
    if (!match(Token::PTCOMMA)) {
        throwParser("Ok pero Falta el ptcomma amig@");
        return false;
    }
    return res;
}


void Parser::parse() {
    // get first token
    current = scanner->nextToken();
    if (check(Token::ERR)) {
        throwParser("Error en scanner - caracter invalido");
        // cout << report << endl;
        resetParser();
        return;
    }

    bool res = parseSentence();

    if (!res) { 
        // cout << report << endl;
        reportFin = report;
        resetParser();
        return;
    }

    if (!match(Token::END)) {
        
        ostringstream os; os << current;
        throwParser("Error: Esperaba fin de input, encontro: " + os.str());
        resetParser();
        return;
    }

    throwParser("Ejecucion completada");
    resetParser();
}
