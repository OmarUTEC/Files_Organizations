#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <functional>

using namespace std;

class coach {
private:
    char code[8];
    char name[36];
    char gender[21];
    char Assistant_Coach[17];
    char country_code[4];
    char country[17];
    char disciplines[21];

public:
    char archivo;
    int next;

    coach() {
        archivo = 'd';
        next = -1;
    }

    void setData(const char* code_, const char* name_, const char* gender_, const char* Assistant_Coach_, const char* country_code_, const char* country_, const char* disciplines_) {
        strncpy(code, code_, 7);
        code[7] = '\0';
        strncpy(name, name_, 35);
        name[35] = '\0';
        strncpy(gender, gender_,20);
        gender[20] = '\0';
        strncpy(Assistant_Coach, Assistant_Coach_, 16);
        Assistant_Coach[16] = '\0';
        strncpy(country_code, country_code_, 3);
        country_code[3] = '\0';
        strncpy(country, country_, 16);
        country[16] = '\0';
        strncpy(disciplines, disciplines_, 20);
        disciplines[20] = '\0';
    }

    void showData() const {
        cout << "Code: " << code << endl;
        cout << "Name: " << name << endl;
        cout << "Gender: " << gender << endl;
        cout << "Assistant Coach: " << Assistant_Coach << endl;
        cout << "Country Code: " << country_code << endl;
        cout << "Country: " << country << endl;
        cout << "Disciplines: " << disciplines << endl;
        cout << "Archivo: " << archivo << endl;
        cout << "Next: " << next << endl;
    }

    const char* getCode() const {
        return code;
    }
};

template <class T, typename TK>
class SequentialFile {
private:
    string mainFile;
    string auxFile;
    std::function<bool(const T&, const TK&)> equal_key;
    std::function<bool(const T&, const T&)> less;

public:
    SequentialFile(string mainFilename, string auxFilename,
                   std::function<bool(const T&, const TK&)> equal_key,
                   std::function<bool(const T&, const T&)> less)
        : mainFile(mainFilename), auxFile(auxFilename), equal_key(equal_key), less(less) {

        fstream FilePrincipal(mainFile, ios::in | ios::binary);
        if (!FilePrincipal.is_open()) {
            ofstream createMainFile(mainFile, ios::out | ios::binary);
            createMainFile.close();
        }
        FilePrincipal.close();

        fstream auxFileStream(auxFile, ios::in | ios::binary);
        if (!auxFileStream.is_open()) {
            ofstream CrearAuxiliar(auxFile, ios::out | ios::binary);
            CrearAuxiliar.close();
        }
        auxFileStream.close();
    }

    T readRecord(int pos, char fileChar) {
        T result;
        fstream file;

        if (fileChar == 'a') {
            file.open(auxFile, ios::in | ios::binary);
        } else if (fileChar == 'd') {
            file.open(mainFile, ios::in | ios::binary);
        }

        if (!file.is_open()) {
            throw runtime_error("No se pudo abrir el archivo");
        }

        file.seekg(pos * sizeof(T), ios::beg);
        file.read((char*)&result, sizeof(T));
        file.close();
        return result;
    }

    vector<T> reorganizar() {
        vector<T> RegisterRecords;
        fstream FilePrincipal(mainFile, ios::in | ios::binary);
        fstream auxFileStream(auxFile, ios::in | ios::binary);

        if (!FilePrincipal.is_open()) {
            ofstream createMainFile(mainFile, ios::out | ios::binary);
            createMainFile.close();
            FilePrincipal.open(mainFile, ios::in | ios::binary);
        }

        if (!auxFileStream.is_open()) {
            ofstream createAuxFile(auxFile, ios::out | ios::binary);
            createAuxFile.close();
            auxFileStream.open(auxFile, ios::in | ios::binary);
        }

        T record;
        while (FilePrincipal.read((char*)&record, sizeof(T))) {
            RegisterRecords.push_back(record);
        }

        while (auxFileStream.read((char*)&record, sizeof(T))) {
            RegisterRecords.push_back(record);
        }

        sort(RegisterRecords.begin(), RegisterRecords.end(), less);

        FilePrincipal.close();
        auxFileStream.close();

        ofstream mainFileOut(mainFile, ios::out | ios::binary | ios::trunc);
        for (const auto& rec : RegisterRecords) {
            mainFileOut.write((char*)&rec, sizeof(T));
        }
        mainFileOut.close();

        ofstream auxFileOut(auxFile, ios::trunc);
        auxFileOut.close();

        return RegisterRecords;
    }

    bool insert(T record) {
        fstream aux(auxFile, ios::app | ios::binary);
        if (!aux.is_open()) {
            throw runtime_error("No se pudo abrir el archivo auxiliar");
        }

        aux.write((char*)&record, sizeof(T));
        aux.close();
        return true;
    }

    bool remove(TK key) {
        fstream main(mainFile, ios::in | ios::out | ios::binary);
        fstream aux(auxFile, ios::in | ios::out | ios::binary);
        if (!main.is_open() || !aux.is_open()) {
            throw runtime_error("No se pudo abrir uno de los archivos");
        }

        T record;
        bool found = false;
        vector<T> newRegisters;

        while (main.read((char*)&record, sizeof(T))) {
            if (equal_key(record, key)) {
                found = true;
            } else {
                newRegisters.push_back(record);
            }
        }

        if (!found) {
            while (aux.read((char*)&record, sizeof(T))) {
                if (equal_key(record, key)) {
                    found = true;
                } else {
                    newRegisters.push_back(record);
                }
            }
        }

        main.close();
        aux.close();

        if (!found) {
            cout << "No se encontró el registro con la clave dada." << endl;
            return false;
        }

        ofstream FileOut(mainFile, ios::out | ios::binary | ios::trunc);
        for (const auto& rec : newRegisters) {
            FileOut.write((char*)&rec, sizeof(T));
        }
        FileOut.close();

        return true;
    }

    vector<T> search(TK key) {
        fstream main(mainFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        vector<T> results;
        T record;

        while (main.read((char*)&record, sizeof(T))) {
            if (equal_key(record, key)) {
                results.push_back(record);
            }
        }

        while (aux.read((char*)&record, sizeof(T))) {
            if (equal_key(record, key)) {
                results.push_back(record);
            }
        }

        main.close();
        aux.close();

        return results;
    }

    vector<T> BuscarRangp(TK key1, TK key2) {
        fstream main(mainFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        vector<T> results;
        T coachRegister;

     // TODO ME DIO SUEÑO :D

        return results;
    }


};
// Comparar dos entrenadores por su código

bool CompararCode(const coach& c1, const coach& c2) {
    return strcmp(c1.getCode(), c2.getCode()) < 0;
}
// Comparar un entrenador con una clave (código)

bool compare_coach_with_key(const coach& c, const char* key) {
    return strcmp(c.getCode(), key) < 0;
}
// Comparar una clave con un entrenador

bool compare_key_with_coach(const char* key, const coach& c) {
    return strcmp(key, c.getCode()) < 0;
}
// Verificar si el código de un entrenador es igual a una clave

bool equal_Key(const coach& c, const char* key) {
    return strcmp(c.getCode(), key) == 0;
}

vector<coach> readCSV(const string& filename) {
    vector<coach> coaches;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo: " << filename << endl;
        return coaches;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string code, name, gender, assistant_coach, country_code, country, disciplines;
        getline(ss, code, ',');
        getline(ss, name, ',');
        getline(ss, gender, ',');
        getline(ss, assistant_coach, ',');
        getline(ss, country_code, ',');
        getline(ss, country, ',');
        getline(ss, disciplines, ',');

        coach objeto1;
        objeto1.setData(code.c_str(), name.c_str(), gender.c_str(), assistant_coach.c_str(), country_code.c_str(), country.c_str(), disciplines.c_str());
        coaches.push_back(objeto1);
    }

    file.close();
    return coaches;
}

int main() {
    SequentialFile<coach, const char*> archivoPrueba("mainFile.dat", "auxFile.dat", equal_Key, CompararCode);

    string filename = "coaches.csv";
    vector<coach> coaches = readCSV(filename);

    for (const auto& c : coaches) {
        archivoPrueba.insert(c);
    }

    vector<coach> sortedCoaches = archivoPrueba.reorganizar();

    for (const auto& c : sortedCoaches) {
        c.showData();
        cout << "-------------------------------" << endl;
    }





    //vector<coach> rangeResults = archivoPrueba.range_search("1533246", "1536328");
    //for (const auto& rec : rangeResults) {
     //   rec.showData();
    //}
    //auto valor= archivoPrueba.search("354664");
    //for (const auto& coach : valor) {
      //  coach.showData();
   // }

   // auto delete_value= archivoPrueba.remove("354664");
   //cout<<" DELETE ? "<<endl;
    //auto valor2= archivoPrueba.search("354664");
    //for (const auto& coach : valor2) {
      //  coach.showData();
    //
/*  todo pediente rango
    auto  arreglo= archivoPrueba.BuscarRangp("9230020","9450042");
    for( auto valores : arreglo) {
        cout<<"......................"<<endl;
        valores.showData();
    }*/
    return 0;
}

