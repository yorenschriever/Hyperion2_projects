#include "platform/includes/webServer.hpp"
#include "platform/includes/webServerResponseBuilder.hpp"
#include "platform/includes/thread.hpp"
#include "platform/includes/utils.hpp"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
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

CompileResponse compileResponse;

int main()
{
    // This project doesn't really need the hyper stack/lib to run, only a webserver.
    // I use the hyper stack here, so you can build/run it like all the other projects.
    // Also, this automatically serves files like preact and monitor which i can reuse.
    // And lastly, it is an easy way to get a backend to build the wasm.
    auto webServer = WebServer::createInstance();

    webServer->addPath("/compile", &compileResponse);

    while (1)
        Thread::sleep(1000);
}