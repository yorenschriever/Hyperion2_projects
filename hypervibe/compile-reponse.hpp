#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>

#include "platform/includes/webServer.hpp"
#include "platform/includes/webServerResponseBuilder.hpp"
#include "readfile.hpp"

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

        system("cd ./compile && ./compile-server.sh");
        
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