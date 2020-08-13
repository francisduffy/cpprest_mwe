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
    MweHandler(const utility::string_t& uri) {
        
        listener_ = std::make_shared<http_listener>(uri, config_);

        // add listener methods
        listener_->support(methods::GET, bind(&MweHandler::handle_get, this, std::placeholders::_1));
        listener_->support(methods::PUT, bind(&MweHandler::handle_put, this, std::placeholders::_1));
        listener_->support(methods::POST, bind(&MweHandler::handle_post, this, std::placeholders::_1));
        listener_->support(methods::DEL, bind(&MweHandler::handle_delete, this, std::placeholders::_1));
    }

    pplx::task<void> open() { return listener_->open(); }
    pplx::task<void> close() { return listener_->close(); }

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

            http_client_config config;
            config.set_timeout(std::chrono::seconds(600));
            http_client client(dataUri, config);

            http_request request(methods::GET);

            http_headers headers;
            headers.add(_XPLATSTR("Accept"), _XPLATSTR("application/json"));
            request.headers() = headers;

            http_response response;
            try {
                response = client.request(request).get();
            } catch (exception& e) {
                cout << "Caught unexpected exception in Retriever: " << e.what() << "." << endl;
                throw;
            }

            // If request was successful.
            status_code code = response.status_code();
            if (code != status_codes::OK) {
                cout << "Status code is not 200." << endl;
                throw;
            }

            response.content_ready().wait();
            cout << "Request was successful." << endl;
            pplx::task<http_response> result = pplx::task_from_result(response);

            pplx::task<web::json::value> promise = result.then([dataUri](web::http::http_response response) {

                utility::string_t result = response.extract_string().get();
                ucout << _XPLATSTR("JsonRetriever got message of length ") << result.length() <<
                    _XPLATSTR(" characters from uri ") << dataUri << "." << std::endl;

                std::error_code errorCode;
                web::json::value val = web::json::value::parse(result, errorCode);
                if (errorCode) {
                    // Just log if error.
                    std::cout << "JSON parsing failed with error code " << errorCode << "." << std::endl;
                }

                return pplx::task_from_result(val);

                });


            // Loop over all data. Don't check types here.
            web::json::value resultBody = promise.get();
            map<pair<string_t, string_t>, double> data;
            for (auto datum : resultBody.as_array()) {
                string_t date = datum.at(_XPLATSTR("date")).as_string();
                string_t id = datum.at(_XPLATSTR("id")).as_string();
                double value = datum.at(_XPLATSTR("value")).as_number().to_double();
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

    std::shared_ptr<web::http::experimental::listener::http_listener> listener_;
    web::http::experimental::listener::http_listener_config config_;
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
    if (argc == 2) {
        hostname = argv[1];
    }

    ostringstream_t oss;
    oss << _XPLATSTR("http://") << hostname << _XPLATSTR(":5003");
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
