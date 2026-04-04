#include "platform/includes/webServer.hpp"
#include "platform/includes/webServerResponseBuilder.hpp"
#include "platform/includes/thread.hpp"
#include "platform/includes/utils.hpp"

#include "compile-reponse.hpp"
#include "generate-response.hpp"

CompileResponse compileResponse;
GenerateResponse generateResponse;

int main()
{
    // This project doesn't really need the hyper stack/lib to run, only a webserver.
    // I use the hyper stack here, so you can build/run it like all the other projects.
    // Also, this automatically serves files like preact and monitor which i can reuse.
    // And lastly, it is an easy way to get a backend to build the wasm.
    auto webServer = WebServer::createInstance();

    webServer->addPath("/api/compile", &compileResponse);
    webServer->addPath("/api/generate", &generateResponse);

    while (1)
        Thread::sleep(1000);
}