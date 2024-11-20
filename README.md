# PROYECTO 1: ORGANIZACIÓN DE ARCHIVOS

| [**Javier Omar Chavarria Humareda**](https://github.com/OmarUTEC) | [**Yacira Nicol Campoverde**](https://github.com/YaciraUTEC) | [**Aldair Sanchez Seminario**](https://github.com/Aldair456) |  |
|-------------------------------------------------------------------|----------------------------------------------------------------|-------------------------------------------------------------------|---------------------------------------------------------------------|

## Tabla de Contenidos
- [Introducción](#introducción)
- [Objetivo](#objetivo)
  - [Objetivo General](#objetivo-general)
  - [Objetivo Específico](#objetivo-específico)
- [Dominio de Datos](#dominio-de-datos)
- [Resultados Esperados al Aplicar Técnicas de Indexación](#resultados-esperados-al-aplicar-técnicas-de-indexación)
- [Técnicas de Indexación](#técnicas-de-indexación)
  - [AVL File Organization](#avl-file-organization)
  - [Sequential File Organization](#sequential-file-organization)
  - [Extendible Hashing](#extendible-hashing)
- [Parser SQL](#sql-parser)
- [Resultados  Experimentales](#resultados-experimentales)
  - [Cuadro comparativo del desempeño de las técnicas](#Cuadro-comparativo-del-desempeño-de-las-tecnicas-de-indexación)
  - [Discusión y Análisis de Resultados](#Discusión-y-análisis-de-resultados)
- [Conclusiones](#conclusiones)

## Introducción
Este proyecto se centra en la integración de datos estructurados y no estructurados, y en el desarrollo de un mini gestor de bases de datos que facilitará la comprensión y la aplicación de técnicas de organización e indexación de archivos.

### OBJETIVO 
`OBJETIVO GENERAL:`  

  Desarrollar un mini sistema de gestor base de datos que integre tanto un modelo relacional basado en 
  tablas como técnicas de recuperación de información. Se aplicarán 3 técnicas de organización de arvhivos: 
  AVL File Organization, Sequential File Organization y Extendible Hashing
  
`OBJETIVO ESPECÍFICO:`  
  - Implementar las 3 técnicas de organización e indexación para desarrollar un mini gestor
  - Implementar las operaciones de búsqueda, búsqueda por rango, añadir y remover
  - Implementar un parser de código SQL
  - Mostrar los resultados de forma interactiva

### DOMINIO DE DATOS
Elegimos 2 datasets: `coaches.csv` y `olympics.csv` con dominos de datos conformados por:  


```cpp
class Coach {
    private:
        char code[8]
        char name[41]
        char gender [7]
        char country [16]
        char disciplines [21]
}
```
```cpp
class Olympics {
     private:
        int player_id
        char Name [30]
        char Sex
        char Team [20]
        char NOC[3]
        int Year
        char Season[6]
        char City[20]
        char Sport [15]
        char Event [40]
        char Medal [8]
}
```
**Los datasets que estamos utilizando son archivos .csv con registros de longitud fija.
Dicho dominio de datos nos ayudará a asegurar que los datos almacenados en un sistema sean coherentes y válidos.**

### RESULTADOS ESPERADOS AL APLICAR TÉCNICAS DE INDEXACIÓN
- Optimizar el tiempo de búsqueda al aplicar el índice B+ Tree y el hashing.
- Optimización del Almacenamiento
- Facilidad de inserción y eliminación
- Acceso eficiente a Rangos de Datos
- Facilitación de consultas SQL

## TÉCNICAS DE INDEXACIÓN - ALGORITMO

1. `AVL File Organization`

   Estructura de datos de árbol binario de búsqueda que mantiene un equilibrio entre sus subárboles, lo que asegura que las operaciones de búsqueda, inserción y eliminación se realicen em un tiempo promedio de O(log n). Es decir combina la eficiencia de un árbol binario de búsqueda con la garantía de un rendimiento equilibrio.

   - `Inserción`
     
     ```cpp
            void insert(Record record) {
            readHead();
            record.left = -1;
            record.right = -1;
            record.height = 0;

            int fatherPos = pos_root;
            stack<Record> fatherStack;
            stack<int> filePos;
            stack<bool> treeTour;
            Record father{};

            // cout << "\nRecorrido del arbol\n";
            while (fatherPos != -1) {
                cout << fatherPos << " _ ";
                readRecord(fatherPos, father);
                fatherStack.push(father);
                filePos.push(fatherPos);
                treeTour.push(father.codigo > record.codigo);
                fatherPos = (treeTour.top()) ? father.left : father.right;
            } cout << "-1 \n";

            int posChild;
            if (next_removed != -1) {
                posChild = next_removed;
                writeNextRemoved(posChild);
            } else posChild = size();
            if (pos_root == -1) writeRoot(posChild);
            writeRecord(posChild, record);

            while(!fatherStack.empty()) {
                father = fatherStack.top();
                fatherStack.pop();
                fatherPos = filePos.top();
                filePos.pop();
                if(treeTour.top()) father.left = posChild;
                else father.right = posChild;
                treeTour.pop();
                updateHeight(fatherPos, father);
                int balance = balancingFactor(father);
                balanceRecord(fatherPos, father, balance);
                if (fatherStack.empty())
                    writeRoot(fatherPos);
                posChild = fatherPos;
            } std::cout << "\nNueva raiz: " << pos_root << '\n' << std::endl;
        }



     ```
     
   - `Búsqueda`
  
     ```cpp
        Record find (TK key) {
            readHead();
            bool found = false;
            int currentPos = pos_root;
            Record record {};
            while(currentPos != -1 && !found) {
                readRecord(currentPos, record);
                found = (key == record.codigo);
                currentPos = (key > record.codigo) ? record.right : record.left;
            }
            if (!found) record = Record{};
            return record;
        };

     ```
   - `Eliminación`
  
     ```cpp
     bool remove(TK key) {
            readHead();
            int currentPos = pos_root, posRemoved = -1;
            bool removed = false, found = false;
            stack<int> posStack;
            stack<Record> nodeStack;
            stack<bool> leftChildStack;
            Record record{};

            while (currentPos != -1 && !found) {
                readRecord(currentPos, record);
                found = (key == record.codigo);
                removed = (record.left == -1 || record.right == -1);
                if (found && removed) {
                    cout << "\nCodigo " << key << " encontrado";
                    int newChild;
                    posRemoved = currentPos;
                    if (record.left == -1 && record.right == -1)
                        newChild = -1;
                    else if (record.left == -1)
                        newChild = record.right;
                    else
                        newChild = record.left;
                    cout << "\nNewChild: " << newChild;
                    if (!posStack.empty()) {
                        int posFather = posStack.top();
                        Record father{};
                        readRecord(posFather, father);
                        if (leftChildStack.top()) father.left = newChild;
                        else father.right = newChild;
                        writeRecord(posFather, father);
                    } else writeRoot(newChild);
                } else {
                    removed = false;
                    posStack.push(currentPos);
                    nodeStack.push(record);
                    leftChildStack.push(key <= record.codigo);
                    currentPos = (key <= record.codigo) ? record.left : record.right;
                }
            }
            if (found && !removed) {
                removed = true;
                Record* temp = &nodeStack.top();
                while (currentPos != -1) {
                    posStack.push(currentPos);
                    readRecord(posStack.top(), record);
                    nodeStack.push(record);
                    leftChildStack.push(false);
                    currentPos = record.right;
                }
                int left = temp->left, right = temp->right, height = temp->height;
                *temp = nodeStack.top();
                temp->left = left;
                temp->right = right;
                temp->height = height;
                record = nodeStack.top();
                nodeStack.pop();
                posRemoved = posStack.top();
                posStack.pop();
                leftChildStack.pop();
            }

            if (removed) {
                updateNextRemoved(posRemoved);
                int posChild;
                if(leftChildStack.top()) posChild = record.left;
                else posChild = record.right;
                while(!nodeStack.empty()) {
                    Record father = nodeStack.top();
                    nodeStack.pop();
                    int fatherPos = posStack.top();
                    posStack.pop();
                    if(leftChildStack.top()) father.left = posChild;
                    else father.right = posChild;
                    leftChildStack.pop();
                    updateHeight(fatherPos, father);
                    int balance = balancingFactor(father);
                    balanceRecord(fatherPos, father, balance);
                    if (nodeStack.empty())
                        writeRoot(fatherPos);
                    posChild = fatherPos;
                }
            }
            return removed;
        };



     ```



     

**COMPLEJIDAD**

| Insert    | Search    | Search Range     | Remove    |
|-----------|-----------|------------------|-----------|
| O(log(n)) | O(log(n)) | O(log(n) * k)    | O(log(n)) |

- n: Número de elementos almacenados
- k: Número de nodos dentro del rango establecido

   
   
2. `Sequential File Organization`
   
   Organiza los datos en un orden lineal, donde los registros se almacenan uno tras otro en un orden lineal, donde los registros se almacenan uno tras otro. Los registros se almacenan en un orden específico, que puede ser determinado por una **clave primaria**.

   - `Inserción`
     
     
      ```cpp
        bool insert(T record) {
        fstream aux(auxFile, ios::app | ios::binary);
        if (!aux.is_open()) {
            throw runtime_error("No se pudo abrir el archivo auxiliar");
        }

        aux.write((char*)&record, sizeof(T));
        aux.close();
        return true;
        }
    
        ```
     
   - `Búsqueda`
  
     
        ```CPP
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
    
        ```


     **Búsqueda por Rango**
    

       ```cpp
      

        vector<T> BuscarRangp(TK key1, TK key2) {
        fstream main(mainFile, ios::in | ios::binary);
        fstream aux(auxFile, ios::in | ios::binary);
        vector<T> results;
        T coachRegister;

        if (!main.is_open() || !aux.is_open()) {
            throw runtime_error("No se pudo abrir uno de los archivos");
        }

        while (main.read((char*)&coachRegister, sizeof(T))) {
            if ((compare_coach_with_key(coachRegister, key2) || strcmp(coachRegister.getCode(), key2) == 0)
                && !compare_coach_with_key(coachRegister, key1)) {
                results.push_back(coachRegister);
                }
        }

        while (aux.read((char*)&coachRegister, sizeof(T))) {
            if ((compare_coach_with_key(coachRegister, key2) || strcmp(coachRegister.getCode(), key2) == 0)
                && !compare_coach_with_key(coachRegister, key1)) {
                results.push_back(coachRegister);
                }
        }

        main.close();
        aux.close();

        return results;

        return results; }

      ```
     
   - `Eliminación`
  
     ```cpp
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


     ```
  
     


  **COMPLEJIDAD**
  | Insert    | Search    | Search Range     | Remove    |
|-----------|-----------|------------------|-----------|
| O(log(n)) | O(log(n)) | O(logn) + O(k)    | O(n) |
  
3. `Extendible Hashing` 
   
   Utiliza una tabla de hash para organizar y acceder a registros de manera eficiente. Está compuesta por un conjunto de **páginas** que contienen punteros a los registros. Cuando la tabla se llena, se puede expandir mediante la creación de nuevas páginas. Al principio solo se utilizan algunos bits de la clave para indexar. A medida que se añaden más registros y se producen colisiones, se pueden usar más bits para dividir las páginas.

   - `Inserción`
     Se utilizará la secuencia D-bit para localizar el bucket correspondiente. Si no se encuentra el             bucket, se procederá a buscar uno con la profundidad local mínima. En caso de que no se encuentre un        bucket adecuado, se creará uno nuevo. Si se encuentra un bucket que no está lleno, se procederá a           insertar el registro. Si el bucket está lleno, se dividirá y se reinsertarán todos los registros en la      nueva estructura. Si ya no es posible incrementar la profundidad, se optará por el encadenamiento para      manejar el desbordamiento.
     

    ```cpp
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


    ```


   
    
   - `Búsqueda`
     Se aplicará la función hash a la clave de búsqueda para obtener la secuencia D-bit. Luego, esta secuencia D-bit se utilizará para hacer coincidir con una entrada en el directorio, lo que nos permitirá acceder al bucket correspondiente y localizar el registro deseado.


   ```cpp

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



   ```

   
     
   - `Eliminación`
     Localizar el buffer y eliminamos el registro, si el bucket queda vacío, puede ser liberado, si es que tienen pocos elementos se procede a mezclar. En ambos casos actualizamos el índice.


    ```cpp
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
    ```
     
 
  **COMPLEJIDAD**
  

| **Insert** | **Search** | **Search Range** | **Remove** |
|------------|------------|------------------|------------|
|  O(log(n))  | O(log(n))  | O(n)             | O(log(n))  |

  
### ANÁLISIS COMPARATIVO
`En base a los accesos en memoria secundaria`


| **Técnica**           | **Inserción**                                                                                                                                                      | **Búsqueda**                                                                                                                                             | **Eliminación**                                                                                                                                            |
|-----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|
| **AVL**        | - En un árbol AVL, cada inserción debe mantener el árbol balanceado. <br> - **Acceso a disco**: Múltiples accesos por las rotaciones necesarias. <br> - **Complejidad**: O(log n). | - Eficiente gracias al equilibrio del árbol. <br> - **Acceso a disco**: Mínimo debido al balance. <br> - **Complejidad**: O(log n). | - Similar a la inserción, con posibles rotaciones. <br> - **Acceso a disco**: Varios accesos posibles. <br> - **Complejidad**: O(log n). |
| **Sequentia File* | - La inserción es costosa, se reorganiza el archivo para mantener el orden. <br> - **Acceso a disco**: Costoso debido a las escrituras. <br> - **Complejidad**: O(log n). | - La búsqueda implica recorrer el archivo secuencialmente. <br> - **Acceso a disco**: Lento para archivos grandes. <br> - **Complejidad**: O(log n). | - Requiere reorganizar el archivo para eliminar huecos. <br> - **Acceso a disco**: Alto costo. <br> - **Complejidad**: O(n). |
| **Hashing Extensible** | - Inserción rápida en promedio. División de bucket si está lleno. <br> - **Acceso a disco**: Rápido salvo en divisiones de buckets. <br> - **Complejidad**: O(1) en promedio. | - Eficiente gracias al acceso directo por hashing. <br> - **Acceso a disco**: Rápido en promedio. <br> - **Complejidad**: O(1) promedio, O(n) con colisiones severas. | - Similar a la inserción. División o fusión de buckets puede incrementar costo. <br> - **Acceso a disco**: Generalmente rápido. <br> - **Complejidad**: O(1) en promedio. |



## SQL PARSER

El parser analiza consultas SQL utilizando un enfoque de análisis descendente recursivo. Trabaja con un scanner que convierte el input en tokens. Las funciones clave, como match(), advance(), y check(), ayudan a recorrer y validar los tokens mientras se almacenan temporalmente atributos y valores en una estructura llamada memoria. El parser maneja sentencias como SELECT, INSERT, DELETE y CREATE, y tiene mecanismos para manejar errores y reiniciar su estado tras cada operación.

```cpp
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

```

###   CONSULTAS


| **Consulta** | **Sentencia**                                                   |
|--------------|------------------------------------------------------------------|
| **SELECT**   | `SELECT * FROM table WHERE column = value`                       |
| **SELECT**   | `SELECT * FROM table WHERE column BETWEEN value1 AND value2`     |
| **CREATE**   | `CREATE INDEX nameIndex ON column USING typeIndex`               |
| **INSERT**   | `INSERT INTO table VALUES (value1, value2, ..., valuen)`         |
| **DELETE**   | `DELETE FROM table WHERE column = value`                         |


## RESULTADOS EXPERIMENTALES

Consultas en nuestro Dataset

```cpp
SELECT FROM coach WHERE DNI = 10961234;
```
```cpp
DELETE FROM coach WHERE DNI = 10961234;
```


### CUADRO COMPARATIVO DEL DESEMPEÑO DE LAS TÉCNICAS DE INDEXACIÓN
`Inserción` `Búsqueda`

**INSERCIÓN**


| **Registros** | **AVL File** | **Sequential File** | **Extendible Hashing** |
|-----------------|--------------|---------------------|------------------------|
| 100             |         |             |              |
| 1000            |     |            |                |


**BÚSQUEDA**


| **Registros** | **AVL File** | **Sequential File** | **Extendible Hashing** |
|-----------------|--------------|---------------------|------------------------|
| 100             |         |             |              |
| 1000            |     |            |                |


### VIDEO EXPLICATIVO
- [Link](https://drive.google.com/file/d/1x5q0wvYmBVo9ryubo3ml9bSXAxjj29-g/view?usp=sharing)
