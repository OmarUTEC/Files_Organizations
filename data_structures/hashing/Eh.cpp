#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "string.h"
#include <cstring>

#include <sstream>
#include <unordered_map>
#include <bitset>

using namespace std;

struct Record {
    char code[8];
    char name[36];
    char gender[21];
    char Assistant_Coach[17];
    char country_code[5];
    char country[17];
    char disciplines[21];

    // Constructor por defecto
    Record() {
        memset(code, 0, sizeof(code));
        memset(name, 0, sizeof(name));
        memset(gender, 0, sizeof(gender));
        memset(Assistant_Coach, 0, sizeof(Assistant_Coach));
        memset(country_code, 0, sizeof(country_code));
        memset(country, 0, sizeof(country));
        memset(disciplines, 0, sizeof(disciplines));
    }

    // Constructor con parámetros
    Record(const char* c, const char* n, const char* g, const char* a, const char* cc, const char* co, const char* d) {
        strncpy(code, c, sizeof(code));
        strncpy(name, n, sizeof(name));
        strncpy(gender, g, sizeof(gender));
        strncpy(Assistant_Coach, a, sizeof(Assistant_Coach));
        strncpy(country_code, cc, sizeof(country_code));
        strncpy(country, co, sizeof(country));
        strncpy(disciplines, d, sizeof(disciplines));

    }

    // Serializar y deserializar como antes
    void serialize(fstream &file) const {
        file.write(code, sizeof(code));
        file.write(name, sizeof(name));
        file.write(gender, sizeof(gender));
        file.write(Assistant_Coach, sizeof(Assistant_Coach));
        file.write(country_code, sizeof(country_code));
        file.write(country, sizeof(country));
        file.write(disciplines, sizeof(disciplines));
    }

    void deserialize(fstream &file) {
        file.read(code, sizeof(code));
        file.read(name, sizeof(name));
        file.read(gender, sizeof(gender));
        file.read(Assistant_Coach, sizeof(Assistant_Coach));
        file.read(country_code, sizeof(country_code));
        file.read(country, sizeof(country));
        file.read(disciplines, sizeof(disciplines));
    }

    // Método para mostrar los datos del registro
     void display() const {
        cout << "Code: " << code << endl;
        cout << "Name: " << name << endl;
        cout << "Gender: " << gender << endl;
        cout << "Assistant Coach: " << Assistant_Coach << endl;
        cout << "Country Code: " << country_code << endl;
        cout << "Country: " << country << endl;
        cout << "Disciplines: " << disciplines << endl;
    }
};


// Definición de la estructura Bucket
struct Bucket {
    vector<Record> records;
    int capacidad;
    int size;

    Bucket(int fb) : capacidad(fb), size(0) {
        records.resize(fb); // Inicializamos el vector con capacidad fb
    }

    size_t sizeInBits() {
        size_t sizeBits = sizeof(this->capacidad) + sizeof(this->size);
        for (const Record& record : this->records) {
            sizeBits += sizeof(record);
        }
        return sizeBits;
    }

    void insert(const Record& record) {
        for (auto& rec : records) {
            if (rec.code[0] == '\0') { // Si encontramos un espacio vacío
                rec = record;
                size++;
                break;
            }
        }
    }

    void serialize(fstream& file) {
        file.write(reinterpret_cast<const char*>(&capacidad), sizeof(capacidad));
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (const Record& record : records) {
            record.serialize(file);
        }
    }

    void deserialize(fstream& file) {
        file.read(reinterpret_cast<char*>(&capacidad), sizeof(capacidad));
        file.read(reinterpret_cast<char*>(&size), sizeof(size));
        for (Record& record : records) {
            record.deserialize(file);
        }
    }
};

// Definición de la estructura IndexEntry
struct IndexEntry {
    string bits; // Representación binaria de la clave
    int local_index;

    IndexEntry(int D) : bits(D, '0'), local_index(0) {}

    void serialize(fstream& file) {
        file.write(reinterpret_cast<const char*>(&local_index), sizeof(local_index));
        file.write(bits.c_str(), bits.size());
    }

    void deserialize(fstream& file, int D) {
        file.read(reinterpret_cast<char*>(&local_index), sizeof(local_index));
        char buffer[1024];
        file.read(buffer, D);
        buffer[D] = '\0';
        bits.assign(buffer);
    }
};

class DynamicHash {
private:
    unordered_map<string, int> posBuckets; // Mapa de posiciones de buckets
    string datafile, indexfile, metadatafile;
    int globalDepth, D, FB;
    vector<IndexEntry> index;

public:
    DynamicHash(string datafile, string indexfile, int D, int FB)
        : datafile(datafile), indexfile(indexfile), D(D), FB(FB), globalDepth(1) {
        initialize();
    }

    void initialize() {
        fstream dataFile(datafile, ios::binary | ios::in | ios::out | ios::app);
        fstream indexFile(indexfile, ios::binary | ios::in | ios::out | ios::app);
        if (!dataFile.is_open() || !indexFile.is_open()) {
            cerr << "Error al abrir los archivos" << endl;
            return;
        }

        // Inicialización de buckets y tabla de índices
        vector<string> binarios;
        generarBinarios(D, "", binarios);

        for (const auto& bits : binarios) {
            IndexEntry entry(D);
            entry.bits = bits;
            entry.local_index = 0; // Inicializamos todos los índices a 0
            index.push_back(entry);
            posBuckets[bits] = entry.local_index;
        }

        // Escribir los buckets iniciales
        for (int i = 0; i < 4; ++i) {
            Bucket bucket(FB);
            bucket.serialize(dataFile);
        }

        // Escribir el índice en el archivo
        for (IndexEntry& entry : index) {
            entry.serialize(indexFile);
        }

        dataFile.close();
        indexFile.close();
    }

    void generarBinarios(int n, string binarioActual, vector<string>& resultados) {
        if (n == 0) {
            resultados.push_back(binarioActual);
            return;
        }
        generarBinarios(n - 1, binarioActual + '0', resultados);
        generarBinarios(n - 1, binarioActual + '1', resultados);
    }

   

    string hashing(const char* code) {
    hash<string> hasher;
    size_t hashValue = hasher(string(code));  // Convertimos el código a una cadena y calculamos el hash
    return bitset<32>(hashValue).to_string().substr(32 - D); // Ajustamos la profundidad D
}


    bool add(const Record& record) {
        string hashBits = hashing(record.code);
        int bucketPos = posBuckets[hashBits];

        fstream dataFileStream(datafile, ios::binary | ios::in | ios::out);
        dataFileStream.seekg(bucketPos);
        Bucket bucket(FB);
        bucket.deserialize(dataFileStream);

        if (bucket.size < bucket.capacidad) {
            bucket.insert(record);
            dataFileStream.seekp(bucketPos);
            bucket.serialize(dataFileStream);
            dataFileStream.close();
            return true;
        } else {
            // Si el bucket está lleno, dividire
            splitBucket(hashBits);
            return add(record); // Intentamos nuevamente
        }
    }

    void splitBucket(const string& hashBits) {
        // Incrementar la profundidad global
        globalDepth++;

        vector<IndexEntry*> entriesToUpdate;
        for (auto& entry : index) {
            if (entry.bits == hashBits) {
                entriesToUpdate.push_back(&entry);
            }
        }

        // Dividir los buckets
        for (auto& entry : entriesToUpdate) {
            int newBucketPos;
            Bucket newBucket1(FB), newBucket2(FB);
            // Reasignar los registros entre los dos nuevos buckets
            // Código para reasignar registros...
            entry->local_index = newBucketPos;
        }
    }

    Record search(const char* code) {
    string hashBits = hashing(code);
    if (posBuckets.find(hashBits) != posBuckets.end()) {
        int bucketPos = posBuckets[hashBits];
        fstream dataFileStream(datafile, ios::binary | ios::in);
        dataFileStream.seekg(bucketPos);
        Bucket bucket(FB);
        bucket.deserialize(dataFileStream);
        dataFileStream.close();

        for (const Record& rec : bucket.records) {
            if (strcmp(rec.code, code) == 0) { // Comparar códigos
                return rec;
            }
        }
    }
    return Record(); // No encontrado
}

    Record remove(const char* code) {
        string hashBits = hashing(code);
        if (posBuckets.find(hashBits) != posBuckets.end()) {
            int bucketPos = posBuckets[hashBits];
            fstream dataFileStream(datafile, ios::binary | ios::in | ios::out);
            dataFileStream.seekg(bucketPos);
            Bucket bucket(FB);
            bucket.deserialize(dataFileStream);

            for (auto& rec : bucket.records) {
                if (rec.code == code) {
                    Record removedRecord = rec;
                    rec.code[0] = '\0';  // Marcar como eliminado
                    bucket.size--;
                    dataFileStream.seekp(bucketPos);
                    bucket.serialize(dataFileStream);
                    dataFileStream.close();
                    return removedRecord;
                }
            }
        }
        return Record(); // No encontrado
    }
};


void loadFromCSV(const string& csvFile, DynamicHash& hashTable) {
    ifstream file(csvFile);

    if (!file.is_open()) {
        cerr << "Error al abrir el archivo CSV." << endl;
        return;
    }

    string line, word;
    int recordCount = 0;  // Contador para limitar los registros
    const int maxRecords = 20;  // Máximo de registros a procesar

    // Leer la primera línea que contiene los encabezados
    getline(file, line);

    // Leer cada línea del archivo CSV y detenerse después de 20 registros
    while (getline(file, line) && recordCount < maxRecords) {
        stringstream ss(line);
        vector<string> fields;

        // Dividir cada campo del CSV usando el punto y coma como delimitador
        while (getline(ss, word, ';')) {
            fields.push_back(word);
        }

        // Verificar que el número de campos en cada línea sea el correcto (7 campos)
        if (fields.size() == 7) {
            Record record;

            // Copiar cada campo en su respectivo atributo del registro
            strncpy(record.code, fields[0].c_str(), sizeof(record.code) - 1);
            strncpy(record.name, fields[1].c_str(), sizeof(record.name) - 1);
            strncpy(record.gender, fields[2].c_str(), sizeof(record.gender) - 1);
            strncpy(record.Assistant_Coach, fields[3].c_str(), sizeof(record.Assistant_Coach) - 1);
            strncpy(record.country_code, fields[4].c_str(), sizeof(record.country_code) - 1);
            strncpy(record.country, fields[5].c_str(), sizeof(record.country) - 1);
            strncpy(record.disciplines, fields[6].c_str(), sizeof(record.disciplines) - 1);

            // Asegurar que cada cadena esté correctamente terminada
            record.code[sizeof(record.code) - 1] = '\0';
            record.name[sizeof(record.name) - 1] = '\0';
            record.gender[sizeof(record.gender) - 1] = '\0';
            record.Assistant_Coach[sizeof(record.Assistant_Coach) - 1] = '\0';
            record.country_code[sizeof(record.country_code) - 1] = '\0';
            record.country[sizeof(record.country) - 1] = '\0';
            record.disciplines[sizeof(record.disciplines) - 1] = '\0';

            // Insertar el registro en la tabla hash
            hashTable.add(record);
            recordCount++;  // Incrementar el contador

            // Mostrar el registro que se acaba de insertar
            cout << "Registro insertado:" << endl;
            record.display();
            cout << "------------------------" << endl;
        } else {
            cerr << "Error: El número de campos en la línea no coincide con el formato esperado." << endl;
        }
    }

    file.close();
}



// Función para realizar pruebas con las funcionalidades de DynamicHash


int main(){
    // Crear el sistema de hashing dinámico
    string datafile = "datafile.bin";
    string indexfile = "indexfile.bin";
    int D = 16;  // Profundidad global inicial de 16 bits
    int FB = 4;  // Capacidad de cada bucket (número máximo de registros por bucket)

    // Inicializar la tabla de hash extendible
    DynamicHash hashTable(datafile, indexfile, D, FB);

    // Cargar los datos desde un archivo CSV (20 registros máximo)
    string csvFile = "coaches.csv";
    loadFromCSV(csvFile, hashTable);

    // Pruebas después de la inserción
    cout << "\n=== Registros insertados desde el CSV ===" << endl;
    const char* codeToSearch = "1533246";  // Ajusta este código según los registros cargados
    cout << "Buscando registro con Code: " << codeToSearch << endl;
    Record foundRecord = hashTable.search(codeToSearch);
    if (strcmp(foundRecord.code, "") != 0) {
        cout << "Registro encontrado: " << endl;
        foundRecord.display();
    } else {
        cout << "Registro con Code " << codeToSearch << " no encontrado." << endl;
    }

    // Eliminar un registro y verificar
    const char* codeToDelete = "1535775";
    cout << "\n=== Eliminando registro con Code: " << codeToDelete << " ===" << endl;
    Record removedRecord = hashTable.remove(codeToDelete);
    if (strcmp(removedRecord.code, "") != 0) {
        cout << "Registro eliminado: " << endl;
        removedRecord.display();
    } else {
        cout << "Registro con Code " << codeToDelete << " no encontrado para eliminar." << endl;
    }

    // Verificar la eliminación
    cout << "\n=== Verificación después de eliminar registro con Code: " << codeToDelete << " ===" << endl;
    foundRecord = hashTable.search(codeToDelete);
    if (strcmp(foundRecord.code, "") != 0) {
        cout << "Error: El registro con Code " << codeToDelete << " sigue presente después de la eliminación." << endl;
    } else {
        cout << "Verificación exitosa: Registro con Code " << codeToDelete << " no encontrado después de la eliminación." << endl;
    }

    // Insertar un nuevo registro
    /*
    Record newRecord("123456", "New Name", "Non-Binary", "New Coach", "GBR", "United Kingdom", "Athletics");
    cout << "\n=== Insertando nuevo registro con Code '123456' ===" << endl;
    hashTable.add(newRecord);
    newRecord.display();

    // Verificar la inserción del nuevo registro
    cout << "\n=== Verificación de nuevo registro con Code '123456' ===" << endl;
    foundRecord = hashTable.search("123456");
    if (strcmp(foundRecord.code, "") != 0) {
        cout << "Nuevo registro encontrado: " << endl;
        foundRecord.display();
    } else {
        cout << "Error: Registro con Code '123456' no encontrado después de la inserción." << endl;
    }
    */
    return 0; 
    }

