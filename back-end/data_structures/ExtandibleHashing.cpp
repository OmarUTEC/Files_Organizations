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
    int id;
    char district[50];
    int year;
    int month;
    int day;
    int vol;
    char street[100];

    // Constructor por defecto
    Record() : id(-1), year(-1), month(-1), day(-1), vol(-1) {
        memset(district, 0, sizeof(district));
        memset(street, 0, sizeof(street));
    }

    // Constructor con ID
    Record(int id) : id(id), year(-1), month(-1), day(-1), vol(-1) {
        memset(district, 0, sizeof(district));
        memset(street, 0, sizeof(street));
    }

    // Serializar y deserializar como antes
    void serialize(fstream &file) const {
        file.write(reinterpret_cast<const char*>(&id), sizeof(id));
        file.write(district, sizeof(district));
        file.write(reinterpret_cast<const char*>(&year), sizeof(year));
        file.write(reinterpret_cast<const char*>(&month), sizeof(month));
        file.write(reinterpret_cast<const char*>(&day), sizeof(day));
        file.write(reinterpret_cast<const char*>(&vol), sizeof(vol));
        file.write(street, sizeof(street));
    }

    void deserialize(fstream &file) {
        file.read(reinterpret_cast<char*>(&id), sizeof(id));
        file.read(district, sizeof(district));
        file.read(reinterpret_cast<char*>(&year), sizeof(year));
        file.read(reinterpret_cast<char*>(&month), sizeof(month));
        file.read(reinterpret_cast<char*>(&day), sizeof(day));
        file.read(reinterpret_cast<char*>(&vol), sizeof(vol));
        file.read(street, sizeof(street));
    }

    // Método para mostrar los datos del registro
    void display()  {
        cout << "ID: " << id << endl;
        cout << "Distrito: " << district << endl;
        cout << "Fecha: " << day << "/" << month << "/" << year << endl;
        cout << "Volumen: " << vol << endl;
        cout << "Calle: " << street << endl;
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
            if (rec.id == -1) { // Si encontramos un espacio vacío
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
        for (int i = 0; i < 2; ++i) {
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

    string hashing(int key) {
        return bitset<32>(key).to_string().substr(32 - D);
    }

    bool add(const Record& record) {
        string hashBits = hashing(record.id);
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

    Record search(int key) {
        string hashBits = hashing(key);
        if (posBuckets.find(hashBits) != posBuckets.end()) {
            int bucketPos = posBuckets[hashBits];
            fstream dataFileStream(datafile, ios::binary | ios::in);
            dataFileStream.seekg(bucketPos);
            Bucket bucket(FB);
            bucket.deserialize(dataFileStream);
            dataFileStream.close();

            for (const Record& rec : bucket.records) {
                if (rec.id == key) {
                    return rec;
                }
            }
        }
        return Record(); // No encontrado
    }

    Record remove(int key) {
        string hashBits = hashing(key);
        if (posBuckets.find(hashBits) != posBuckets.end()) {
            int bucketPos = posBuckets[hashBits];
            fstream dataFileStream(datafile, ios::binary | ios::in | ios::out);
            dataFileStream.seekg(bucketPos);
            Bucket bucket(FB);
            bucket.deserialize(dataFileStream);

            for (auto& rec : bucket.records) {
                if (rec.id == key) {
                    Record removedRecord = rec;
                    rec.id = -1;  // Marcar como eliminado
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




// Función para cargar datos desde un archivo CSV
void loadFromCSV(const string& csvFile, DynamicHash& hashTable) {
    ifstream file(csvFile);

    if (!file.is_open()) {
        cerr << "Error al abrir el archivo CSV." << endl;
        return;
    }

    string line, word;
    // Leer la primera línea que contiene los encabezados
    getline(file, line);

    // Leer cada línea del archivo CSV
    while (getline(file, line)) {
        stringstream ss(line);
        vector<string> fields;

        while (getline(ss, word, ',')) {
            fields.push_back(word);
        }

        if (fields.size() == 7) {
            Record record;
            record.id = stoi(fields[0]);
            strcpy(record.district, fields[1].c_str());
            record.year = stoi(fields[2]);
            record.month = stoi(fields[3]);
            record.day = stoi(fields[4]);
            record.vol = stoi(fields[5]);
            strcpy(record.street, fields[6].c_str());

            // Insertar el registro en la tabla hash
            cout << "Insertando registro desde CSV..." << endl;
            hashTable.add(record);
            record.display();
            cout << "---------------------------" << endl;
        }
    }

    file.close();
}

// Función para realizar pruebas con las funcionalidades de DynamicHash
void testHashTable(DynamicHash& hashTable) {
    // Buscar registros insertados
    cout << "\n=== Buscando registros insertados ===" << endl;
    for (int id = 1; id <= 4; ++id) {
        cout << "Buscando registro con ID: " << id << endl;
        Record foundRecord = hashTable.search(id);
        if (foundRecord.id != -1) {
            cout << "Registro encontrado: " << endl;
            foundRecord.display();
        } else {
            cout << "Registro con ID " << id << " no encontrado." << endl;
        }
        cout << "===========================" << endl;
    }

    // Eliminar un registro y verificar
    cout << "\n=== Eliminando registro con ID 2 ===" << endl;
    int deleteId = 2;
    Record removedRecord = hashTable.remove(deleteId);
    if (removedRecord.id != -1) {
        cout << "Registro eliminado: " << endl;
        removedRecord.display();
    } else {
        cout << "Registro con ID " << deleteId << " no encontrado para eliminar." << endl;
    }
    cout << "===========================" << endl;

    // Verificación después de eliminación
    cout << "\n=== Verificación después de eliminar registro con ID 2 ===" << endl;
    Record foundRecord = hashTable.search(deleteId);
    if (foundRecord.id != -1) {
        cout << "Error: El registro con ID " << deleteId << " sigue presente después de la eliminación." << endl;
    } else {
        cout << "Verificación exitosa: Registro con ID " << deleteId << " no encontrado después de la eliminación." << endl;
    }
    cout << "===========================" << endl;

    // Insertar un nuevo registro
    Record newRecord(5);
    strcpy(newRecord.district, "District E");
    newRecord.year = 2024;
    newRecord.month = 9;
    newRecord.day = 12;
    newRecord.vol = 500;
    strcpy(newRecord.street, "Street 5");

    cout << "\n=== Insertando nuevo registro con ID 5 ===" << endl;
    hashTable.add(newRecord);
    newRecord.display();
    cout << "===========================" << endl;

    // Verificar la inserción del nuevo registro
    cout << "\n=== Verificación de nuevo registro con ID 5 ===" << endl;
    foundRecord = hashTable.search(5);
    if (foundRecord.id != -1) {
        cout << "Nuevo registro encontrado: " << endl;
        foundRecord.display();
    } else {
        cout << "Error: Registro con ID 5 no encontrado después de la inserción." << endl;
    }
    cout << "===========================" << endl;
}

int main() {
    // Crear el sistema de hashing dinámico
    string datafile = "datafile.bin";
    string indexfile = "indexfile.bin";
    int D = 3;  // Profundidad global inicial de 3 bits
    int FB = 2; // Capacidad de cada bucket (número máximo de registros por bucket)

    // Inicializar la tabla de hash extendible
    DynamicHash hashTable(datafile, indexfile, D, FB);

    // Cargar los datos desde un archivo CSV
    string csvFile = "records.csv";
    loadFromCSV(csvFile, hashTable);

    // Ejecutar pruebas para verificar las funcionalidades
    testHashTable(hashTable);

    return 0;
}

