#pragma once
#include "hyperion.hpp"
#include "webServer.hpp"
#include "webServerResponseBuilder.hpp"

// Front view
const char viewParams[] = "\n\
export const viewParams = {\n\
    clearColor: [0.1, 0.1, 0.1, 1.0],\n\
    fieldOfView: (80 * Math.PI) / 180,\n\
    gridZ: -0.35,\n\
    transform: [\n\
        {\n\
            //move 2 units back\n\
            type:'translate',\n\
            amount: [0,-0.2,-2.0]\n\
        },\n\
        {\n\
            // look slightly from below\n\
            type:'rotate',\n\
            amount: (_t)=>-0.0,\n\
            vector:[1,0,0]\n\
        },\n\
        {\n\
            //rotate the scene\n\
            type:'rotate',\n\
            amount: (t)=>0,\n\
            vector: [0,1,0]\n\
        }\n\
    ]\n\
};\n";

class ViewParams: public WebServerResponseBuilder
{
    public:
    void build(Writer write, void* userData) override
    {
        write(viewParams,sizeof(viewParams)-1, userData);
    }
};

auto vp = new ViewParams();

void setViewParams(Hyperion *hyp)
{
    hyp->webServer->addPath("/monitor/view-params.js",vp);
}
