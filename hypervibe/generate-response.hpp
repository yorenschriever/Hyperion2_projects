
#include <iostream>
#include <sstream>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include "misc/cjson/cJSON.h"

#include "platform/includes/webServer.hpp"
#include "platform/includes/webServerResponseBuilder.hpp"

using namespace std;
using namespace boost::beast;
using namespace boost::asio;

std::string jsonEscape(std::string input)
{
    std::string output;
    for (char c : input)
    {
        switch (c)
        {
        case '"':
            output += "\\\"";
            break;
        case '\\':
            output += "\\\\";
            break;
        case '\n':
            output += "\\n";
            break;
        case '\r':
            output += "\\r";
            break;
        case '\t':
            output += "\\t";
            break;
        default:
            output += c;
            break;
        }
    }
    return output;
}

std::string cleanString(std::string input)
{
    std::string output;

    // replace \n with an actual newline, \t with an actual tab, etc.
    // also remove occurences of ```
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input.substr(i, 3) == "```")
        {
            i += 2;
        }
        else
        {
            output += input[i];
        }
    }

    return output;
}

std::string readFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Unable to open file: " << filename << endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

class GenerateResponse : public WebServerResponseBuilder
{
public:
    void build(WebServerResponseBuilder::Writer write, std::string postData, void *userData) override
    {

        boost::system::error_code ec;

        io_service svc;

        const string host = "api.anthropic.com";
        const string path = "/v1/messages";
        const string port = "443";

        std::string request(
            "{"
            "    \"model\": \"claude-opus-4-6\","
            "    \"max_tokens\": 20000,"
            "    \"temperature\": 0.5,"
            "    \"system\": \"" +
            jsonEscape(readFile("system-prompt.md")) +
            "\","
            "    \"messages\": ["
            "        {"
            "        \"role\": \"user\","
            "        \"content\": ["
            "            {"
            "            \"type\": \"text\","
            "            \"text\": \"" +
            jsonEscape(postData) +
            "\""
            "            }"
            "        ]"
            "        }"
            "    ],"
            "    \"thinking\": {"
            "        \"type\": \"disabled\""
            "    },"
            "    \"output_config\": {"
            "        \"effort\": \"high\""
            "    }"
            "}\r\n");

        // printf("Request body: %s\n", request.c_str());

        ssl::context ctx(ssl::context::sslv23_client);
        ssl::stream<ip::tcp::socket> ssocket = {svc, ctx};
        ip::tcp::resolver resolver(svc);
        auto it = resolver.resolve(host, port);

        // required for macos.
        ctx.load_verify_file("/private/etc/ssl/cert.pem");
        ctx.set_verify_mode(ssl::verify_peer);
        SSL_set_tlsext_host_name(ssocket.native_handle(), host.c_str());

        // connect(ssocket.lowest_layer(), it);
        // ssocket.handshake(ssl::stream_base::handshake_type::client);
        // http::request<http::string_body> req{http::verb::post, path, 11};
        // req.set(http::field::host, host);
        // req.set(http::field::content_type, "application/json");
        // req.set("x-api-key", readFile("api.key"));
        // req.set("anthropic-version", "2023-06-01");
        // req.body() = request;
        // req.prepare_payload();

        // http::write(ssocket, req);

        // printf("Request sent, waiting for response...\n");

        // http::response<http::string_body> res;
        // flat_buffer buffer;
        // http::read(ssocket, buffer, res);
        // std::string body = res.body();
        // ssocket.lowest_layer().close();

        std::string body = readFile("example-response.json");

        printf("Response received, done.\n");

        cJSON *parsed = cJSON_Parse(body.c_str());
        cJSON *contentNodes = cJSON_GetObjectItem(parsed, "content");
        cJSON *FirstContentNode = cJSON_GetArrayItem(contentNodes, 0);
        auto textNode = cJSON_GetObjectItem(FirstContentNode, "text")->valuestring;

        auto formatted = cleanString(textNode);

        printf("Formatted response: %s\n", formatted.c_str());

        write(formatted.c_str(), formatted.size(), userData);
    }
};