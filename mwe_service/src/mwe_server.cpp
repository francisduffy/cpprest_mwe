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
using namespace web::http::experimental::listener;
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

// Used below to handle incoming requests. Only POST is implemented.
class MweHandler {
public:
    MweHandler(const utility::string_t& uri) : listener_(uri) {
        
        // add listener methods
        listener_.support(methods::GET, bind(&MweHandler::handle_get, this, std::placeholders::_1));
        listener_.support(methods::PUT, bind(&MweHandler::handle_put, this, std::placeholders::_1));
        listener_.support(methods::POST, bind(&MweHandler::handle_post, this, std::placeholders::_1));
        listener_.support(methods::DEL, bind(&MweHandler::handle_delete, this, std::placeholders::_1));
    }

    pplx::task<void> open() { return listener_.open(); }
    pplx::task<void> close() { return listener_.close(); }

private:
    void handle_get(web::http::http_request message) {
        message.reply(status_codes::NotImplemented);
    }

    void handle_put(web::http::http_request message) {
        message.reply(status_codes::NotImplemented);
    }

    void handle_post(web::http::http_request message) {
        try {

            // Try to extract message body.
            web::json::value body;
            try {
                body = message.extract_json().get();
            } catch (exception& e) {
                cout << "Post Exception when extracting JSON from http_request " << e.what() << endl;
                message.reply(status_codes::BadRequest);
                return;
            }

            // Get dataUri from message body
            string_t dataUri;
            if (body.has_field(_XPLATSTR("dataUri"))) {
                const web::json::value& s = body.at(_XPLATSTR("dataUri"));
                if (s.is_string()) {
                    dataUri = s.as_string();
                }
            }

            // Check dataUri is not empty
            if (dataUri.empty()) {
                cout << "Post body needs a 'dataUri' field." << endl;
                message.reply(status_codes::BadRequest);
                return;
            }

            // Fetch data from dataUri
            ucout << _XPLATSTR("Start requesting data from: ") << dataUri << endl;

            map<pair<string_t, string_t>, double> data;
            for (size_t i = 0; i < 5000; ++i) {
                string_t date = _XPLATSTR("2020-08-11");
                ostringstream_t oss;
                oss << _XPLATSTR("xxxxx_") << i;
                string_t id = oss.str();
                double value = 1.5;
                data[make_pair(date, id)] = value;
            }

            cout << "Data map is now of size " << data.size() << endl;
            ucout << _XPLATSTR("Finished requesting data from: ") << dataUri << endl;

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

    web::http::experimental::listener::http_listener listener_;
};

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
    MweHandler mweHandler(address);

    try {
        mweHandler.open().wait();
        ucout << _XPLATSTR("Listening for requests at ") << address << std::endl;
        cout << "Press CTRL+C to exit." << std::endl;
        InterruptHandler::waitForUserInterrupt();
        mweHandler.close().wait();
    } catch (const std::exception& e) {
        cerr << "Hit an error." << std::endl;
        cerr << "Error message: " << e.what() << std::endl;
    } catch (...) {
        cerr << "Hit an error." << std::endl;
    }
    
    return 0;
}
