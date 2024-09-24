#include "Parser.hpp"
#include <iostream>
using namespace std;

int main() {
    cout << "-- Ejecutando interfaz SQL --\n";
    string query;
    Scanner scanner;
    Parser parser(&scanner);
    while (true) {
        cout << ">> ";
        getline(cin, query);
        if (query == "exit") break;
        parser.setInput(query);
        cout << endl;
        parser.parse();
        cout << parser.reportParse() << endl;
        cout << endl;
    }
}
