#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <stack>
#include <queue>

using namespace std;

struct Record {
  int codigo{};
  char nombre[12]{};
  char apellido[12]{};
  int ciclo{};
  int left{};
  int right{};
  int height{};

  void showData() {
    cout << "\nCodigo: " << codigo;
    cout << "\nNombre: " << nombre;
    cout << "\nApellido: " << apellido;
    cout << "\nCiclo : " << ciclo;
    cout << "\nLeft : " << left;
    cout << "\nRight : " << right;
    cout << "\nHeight : " << height;
    cout << '\n';
  }
  bool operator==(const Record& other) const {
    return codigo == other.codigo && strcmp(nombre, other.nombre) == 0 && strcmp(apellido, other.apellido) == 0 && ciclo == other.ciclo;
  }
  Record& operator = (const Record& other);
};

Record &Record::operator=(const Record &other) {
    this->codigo = other.codigo;
    strcpy(this->nombre, other.nombre);
    strcpy(this->apellido, other.apellido);
    this->ciclo = other.ciclo;
    this->left = other.left;
    this->right = other.right;
    this->height = other.height;
    return *this;
}

template <typename TK>
class AVLFile {
    private:
        string filename {};
        int pos_root {};
        int next_removed {};
    public:
        explicit AVLFile (string f) : filename(std::move(f)), pos_root(-1) {
            std::ifstream file (this->filename, std::ios::binary);
            if (!file.is_open()) {
                std::cout << "Creando nuevo archivo." << std::endl;
                std::ofstream newFile (this->filename, std::ios::binary);
                pos_root = -1;
                next_removed = -1;
                newFile.seekp(0);
                newFile.write(reinterpret_cast<const char*>(&pos_root), sizeof(pos_root));
                newFile.write(reinterpret_cast<const char*>(&next_removed), sizeof(next_removed));
                newFile.close();
            } else std::cout << "Archivo disponible" << std::endl;
            file.close();
        };

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

        void print() {
            readHead();
            ifstream file(this->filename, ios::binary);
            if(!file.is_open()) {
                std::cerr << "No se pudo abrir el archivo" << std::endl;
                return;
            }
            queue<int> recordQueue;
            if(pos_root >= 0) recordQueue.push(pos_root);
            while(!recordQueue.empty()) {
                Record r;
                int pos = recordQueue.front();
                recordQueue.pop();
                file.seekg(pos * sizeof(Record) + sizeof(pos_root) + sizeof(next_removed), ios::beg);
                file.read((char*) &r, sizeof(Record));
                cout  << '\n' << "Posicion de archivo " << pos;
                r.showData();
                if(r.left != -1) recordQueue.push(r.left);
                if(r.right != -1) recordQueue.push(r.right);
            }
            file.close();
        }

        vector<Record> inorder() {
            readHead();
            std::vector<Record> result;
            ifstream file(this->filename, ios::binary);
            if(!file.is_open()) {
                std::cerr << "No se pudo abrir el archivo" << std::endl;
                return {};
            }
            stack<int> posStack;
            stack<Record*> nodeStack;
            int posCurrent = pos_root;
            while(posCurrent != -1) {
                posStack.push(posCurrent);
                nodeStack.push(new Record);
                readRecord(posStack.top(), *nodeStack.top());
                posCurrent = nodeStack.top()->left;
            }
            // int iter = 1;
            while(!nodeStack.empty()) {
                /*
                cout << "\nIteracion " << iter;
                iter++;
                cout << "\nPosicion en el archivo:" << posStack.top();
                nodeStack.top()->showData();
                */
                result.push_back(*nodeStack.top());
                posCurrent = nodeStack.top()->right;
                delete nodeStack.top();
                nodeStack.pop();
                posStack.pop();

                while(posCurrent != -1) {
                    posStack.push(posCurrent);
                    nodeStack.push(new Record);
                    readRecord(posStack.top(), *nodeStack.top());
                    posCurrent = nodeStack.top()->left;
                }
            }
            file.close();
            return result;
        };

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

        int size() {
            ifstream file(this->filename, ios::binary);
            if(!file.is_open())
                throw ("No se pudo abrir el archivo");
            file.seekg(0, ios::end);
            long long total_bytes = file.tellg();
            file.close();
            return int((total_bytes - sizeof(pos_root) - sizeof(next_removed)) / sizeof(Record));
        }

        void readRecord(int pos, Record& r) {
            ifstream file(this->filename, ios::binary);
            if(!file.is_open())
                throw ("No se pudo abrir el archivo");
            file.seekg(pos * sizeof(Record) + sizeof(pos_root) + sizeof(next_removed), ios::beg);
            file.read((char*) &r, sizeof(Record));
            file.close();
        }

        void writeRecord(int pos, Record& r) {
            ofstream file(this->filename, ios::in | ios::out | ios::binary);
            if(!file.is_open())
                throw ("No se pudo abrir el archivo");
            file.seekp(pos * sizeof(Record) + sizeof(pos_root) + sizeof(next_removed), ios::beg);
            file.write((char*) &r, sizeof(Record));
            file.close();
        }

        void updateHeight(int pos, Record& r) {
            int left = -1, right = -1;
            Record child {};
            if(r.left != -1) {
                readRecord(r.left, child);
                left = child.height;
            }
            if(r.right != -1) {
                readRecord(r.right, child);
                right = child.height;
            }
            r.height = 1 + int(max(double(left),double(right)));
            writeRecord(pos,r);
            std::cout << "\nActualización en el Registro " << pos;
            std::cout  << '\n' <<" - Left: " << r.left;
            std::cout  << '\n' <<" - Right: " << r.right << '\n';
        };

        int balancingFactor(Record& r) {
            Record child {};
            int leftHeight = -1, rightHeight = -1;
            if(r.left != -1) {
                readRecord(r.left, child);
                leftHeight = child.height;
            }
            if(r.right != -1) {
                readRecord(r.right, child);
                rightHeight = child.height;
            }
            return leftHeight - rightHeight;
        }

        void readHead() {
            std::ifstream file (this->filename, std::ios::binary);
            if(!file.is_open())
                throw("El archivo no fue encontrado");
            file.read(reinterpret_cast<char*>(&pos_root), sizeof(pos_root));
            file.read(reinterpret_cast<char*>(&next_removed), sizeof(next_removed));
            file.close();
        }

        void writeRoot(int root) {
            pos_root = root;
            std::ofstream file (this->filename, std::ios::out | std::ios::binary | std::ios::trunc);
            file.seekp(0);
            file.write(reinterpret_cast<const char*>(&pos_root), sizeof(pos_root));
            file.close();
        };

        void updateNextRemoved (int pos) {
            readHead();
            std::ofstream file (this->filename, ios::in | ios::out | ios::binary);
            file.seekp(sizeof(pos_root));
            file.write(reinterpret_cast<const char*>(&pos), sizeof(next_removed));
            file.seekp(pos * sizeof(Record) + sizeof(pos_root) + sizeof(next_removed));
            file.write(reinterpret_cast<const char*>(&next_removed), sizeof(next_removed));
            std::string emptyBlock(sizeof(Record) - sizeof(next_removed), '\0');
            file.write(emptyBlock.c_str(), sizeof(Record) - sizeof(next_removed));
            file.close();
        }

        void writeNextRemoved(int pos) {
            int next;
            std::ifstream file1(this->filename, std::ios::binary);
            if (!file1.is_open())
                throw("No se pudo abrir el archivo");
            file1.seekg(pos * sizeof(Record) + sizeof(pos_root) + sizeof(next_removed), std::ios::beg);
            file1.read(reinterpret_cast<char*>(&next), sizeof(next));
            file1.close();
            std::ofstream file (this->filename, ios::in | ios::out | ios::binary);
            next_removed = next;
            file.seekp(sizeof(pos_root));
            file.write(reinterpret_cast<const char*>(&next_removed), sizeof(next_removed));
            file.close();
        };

        void balanceRecord(int& pos, Record& r, int balance) {
            int posTemp = pos;
            Record temp = r;
            int posTemp2;
            Record temp2;
            if (balance > 1) {
                cout << "\nRotacion a la derecha\n";
                pos = r.left;
                readRecord(pos, r);
                if (balancingFactor(r) >= 0) {
                    temp.left = r.right;
                    r.right = posTemp;
                    updateHeight(posTemp,temp);
                    updateHeight(pos, r);
                } else {
                    posTemp2 = pos;
                    temp2 = r;
                    pos = r.right;
                    readRecord(pos, r);
                    temp.left = r.right;
                    temp2.right = r.left;
                    r.right = posTemp;
                    r.left = posTemp2;
                    updateHeight(posTemp, temp);
                    updateHeight(posTemp2, temp2);
                    updateHeight(pos, r);
                }
            }
            if (balance < -1) {
                cout << "\nRotacion a la izquierda\n";
                pos = r.right;
                readRecord(pos, r);
                if (balancingFactor(r) <= 0) {
                    temp.right = r.left;
                    r.left = posTemp;
                    updateHeight(posTemp,temp);
                    updateHeight(pos, r);
                } else {
                    posTemp2 = pos;
                    temp2 = r;
                    pos = r.left;
                    readRecord(pos, r);
                    temp.right = r.left;
                    temp2.left = r.right;
                    r.left = posTemp;
                    r.right = posTemp2;
                    updateHeight(posTemp, temp);
                    updateHeight(posTemp2, temp2);
                    updateHeight(pos, r);
                }
            }
        };
};


vector<Record> leerCSV(const string& filename) {
  ifstream file(filename);
  vector<Record> records;
  string line;

  if (!file.is_open()) {
    cerr << "No se pudo abrir el archivo.\n";
    return records;
  }

  getline(file, line);
  while (getline(file, line)) {
    stringstream ss(line);
    string token;
    Record record;

    getline(ss, token, ',');
    record.codigo = stoi(token);

    getline(ss, token, ',');
    strncpy(record.nombre, token.c_str(), sizeof(record.nombre));
    record.nombre[sizeof(record.nombre) - 1] = '\0';  // Asegurar el null-termination

    getline(ss, token, ',');
    strncpy(record.apellido, token.c_str(), sizeof(record.apellido));
    record.apellido[sizeof(record.apellido) - 1] = '\0';  // Asegurar el null-termination

    getline(ss, token, ',');
    record.ciclo = stoi(token);

    records.push_back(record);
  }
  file.close();
  return records;
}

void readFile(string filename){
  AVLFile<int> file(filename);
  std::cout << "Numero de registros en el archivo: " << file.size() << std::endl;
  cout<<"------------------------------------------\n";
  vector<Record> records = leerCSV("datos.csv");
  bool passed = true;
  for (auto& record : records) {
    Record r = file.find(record.codigo);
    if (!(r == record)){
      passed = false;
      cout << "Error en el record con codigo: " << record.codigo << "\n";
      cout << "Se esperaba: \n";
      record.showData();
      cout << "\nSe obtuvo: \n";
      r.showData();
      cout << "\n";
      break;
    }
  }
  if (passed)
    cout << "Todos los records fueron leidos correctamente\n";
}

int main() {
    // El programa puede encontrar el los registros
    // Sin embargo, no he probado casos de falla,
    // es decir, intentar encontrar un registro con un codigo que no está en el archivo
    // readFile("avl_data.dat");

    // Asegurate de eliminar el archivo "avl_data.dat" antes de ejecutar este codigo
    // El programa no está preparado para registros con codigo duplicado

    AVLFile<int> file("avl_data3.dat");
    vector <Record> records = leerCSV("../../dataset/coaches.csv");
    for (auto& record : records) {
        file.insert(record);
    }
    file.print();
    file.remove(481);
    file.remove(500);
    std::cout << "-----------------------------------\n";
    file.print();
    /*
    AVLFile<int> file("avl_data.dat");
    file.print();
     */

    // Asegurate que el archivo tenga data antes de ejecutarlo
    /*
    AVLFile<int> file("avl_data.dat");
    cout << "Numero de registros: " << file.size() << '\n';
    file.inorder();
    */
    return 0;
}
