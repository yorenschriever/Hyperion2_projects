#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>

#include "platform/includes/webServer.hpp"
#include "platform/includes/webServerResponseBuilder.hpp"

using namespace std;

class CompileResponse: public WebServerResponseBuilder
{
public:
    void build(WebServerResponseBuilder::Writer write, std::string postData, void * userData) override
    {
        ofstream myfile;
        myfile.open ("compile/pattern.hpp");
        myfile << postData;
        myfile.close();





        std::array<char, 1024> buffer;
        std::string result;
        FILE* pipe = popen("cd ./compile && ./compile-server.sh 2>&1", "r");
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result += buffer.data();
        }
        int closeResult = pclose(pipe);
        if (closeResult != 0) {
            cerr << "Error: compile-server.sh exited with code " << closeResult << endl;
            write(result.c_str(), result.size(), userData);
            return;
        }




        streampos size;
        char * memblock;

        ifstream file ("compile/pattern.wasm", ios::in|ios::binary|ios::ate);
        if (!file.is_open())
            return;

        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();

        write(memblock, size, userData);

        delete[] memblock;
    }
};