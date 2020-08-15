#ifdef _WIN32
#include <SDKDDKVer.h>
#endif

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <signal.h>
#include <thread>

using utility::ostringstream_t;
using utility::string_t;
using web::http::client::http_client_config;
using web::http::client::http_client;
using web::http::http_request;
using web::http::http_response;
using web::http::status_code;
using web::http::status_codes;
using web::http::method;
using web::http::methods;
using web::http::http_headers;
using web::http::experimental::listener::http_listener;
using web::uri;

using std::bind;
using std::cerr;
using std::condition_variable;
using std::cout;
using std::endl;
using std::exception;
using std::map;
using std::mutex;
using std::pair;
using std::string;
using std::unique_lock;
using std::vector;

static condition_variable _condition;
static mutex _mutex;

// Used below to close server when ctrl+c is sent.
class InterruptHandler {
public:
    static void hookSIGINT() {
        signal(SIGINT, handleUserInterrupt);
    }

    static void handleUserInterrupt(int signal) {
        if (signal == SIGINT) {
            _condition.notify_one();
        }
    }

    static void waitForUserInterrupt() {
        unique_lock<mutex> lock{ _mutex };
        _condition.wait(lock);
        lock.unlock();
    }
};

class ContainerWrapper {
public:
    ~ContainerWrapper() {
        cout << "Destructor: data map is of size " << data.size() << endl;
    }

    map<pair<string_t, string_t>, double> data;
};

void handle_get(web::http::http_request message) {
    message.reply(status_codes::NotImplemented);
}

void handle_put(web::http::http_request message) {
    message.reply(status_codes::NotImplemented);
}

void handle_post(web::http::http_request message) {

    try {

        // Fetch data from dataUri
        ucout << _XPLATSTR("Start adding data.") << endl;

        ContainerWrapper cw;
        for (size_t i = 0; i < 5000; ++i) {
            string_t date = _XPLATSTR("2020-08-11");
            ostringstream_t oss;
            oss << _XPLATSTR("xxxxx_") << i;
            string_t id = oss.str();
            double value = 1.5;
            cw.data[make_pair(date, id)] = value;
        }

        cout << "Data map is now of size " << cw.data.size() << endl;

        unsigned pause = 3;
        cout << "Sleep for " << pause << " seconds." << endl;
        std::this_thread::sleep_for(std::chrono::seconds(pause));

        message.reply(status_codes::OK);

    } catch (exception& e) {
        cerr << "Post Exception " << e.what() << endl;
        throw;
    } catch (...) {
        cerr << "Post Exception (unhandled)" << endl;
        throw;
    }
}

void handle_delete(web::http::http_request message) {
    message.reply(status_codes::NotImplemented);
}

#ifdef _WIN32
int wmain(int argc, wchar_t *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    InterruptHandler::hookSIGINT();
    
    // Can switch host with command line argument
    string_t hostname = _XPLATSTR("localhost");
    if (argc >= 2) {
        hostname = argv[1];
    }
    
    // If second command line variable is provided, assume port. No checks.
    string_t port(_XPLATSTR("5003"));
    if (argc == 3) {
        port = argv[2];
    }

    ostringstream_t oss;
    oss << _XPLATSTR("http://") << hostname << _XPLATSTR(":") << port;
    string_t address = oss.str();

    // Create the listener and add the supported methods.
    http_listener listener(address);
    listener.support(methods::GET, handle_get);
    listener.support(methods::PUT, handle_put);
    listener.support(methods::POST, handle_post);
    listener.support(methods::DEL, handle_delete);

    try {
        listener.open().wait();
        ucout << _XPLATSTR("Listening for requests at ") << address << std::endl;
        cout << "Press CTRL+C to exit." << std::endl;
        InterruptHandler::waitForUserInterrupt();
        listener.close().wait();
    } catch (const std::exception& e) {
        cerr << "Hit an error." << std::endl;
        cerr << "Error message: " << e.what() << std::endl;
    } catch (...) {
        cerr << "Hit an error." << std::endl;
    }
    
    return 0;
}
