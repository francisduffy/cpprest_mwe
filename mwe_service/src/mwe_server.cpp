#include <httplib.h>
#include <iostream>
#include <sstream>

using httplib::Request;
using httplib::Response;
using httplib::Server;

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

class ContainerWrapper {
public:
    ~ContainerWrapper() {
        cout << "Destructor: data map is of size " << data.size() << endl;
    }

    map<pair<string, string>, double> data;
};

void handle_get(const Request& req, Response& res) {
    res.status = 501;
}

void handle_put(const Request& req, Response& res) {
    res.status = 501;
}

void handle_post(const Request& req, Response& res) {

    try {

        // Fetch data from dataUri
       cout << "Start adding data." << endl;

        ContainerWrapper cw;
        for (size_t i = 0; i < 5000; ++i) {
            string date = "2020-08-11";
            string id = "xxxxx_" + std::to_string(i);
            double value = 1.5;
            cw.data[make_pair(date, id)] = value;
        }

        cout << "Data map is now of size " << cw.data.size() << endl;

        unsigned pause = 3;
        cout << "Sleep for " << pause << " seconds." << endl;
        std::this_thread::sleep_for(std::chrono::seconds(pause));

        res.status = 200;

    } catch (exception& e) {
        cerr << "Post Exception " << e.what() << endl;
        throw;
    } catch (...) {
        cerr << "Post Exception (unhandled)" << endl;
        throw;
    }
}

void handle_delete(const Request& req, Response& res) {
    res.status = 501;
}

int main(int argc, char *argv[])
{
    // Can switch host with command line argument
    const char* host = "localhost";
    if (argc >= 2) {
        host = argv[1];
    }
    
    // If second command line variable is provided, assume port. No checks.
    int port = 5003;
    if (argc == 3) {
        std::stringstream strPort;
        strPort << argv[2];
        strPort >> port;
    }

    // Create the listener and add the supported methods.
    Server svr;

    svr.Get("/", handle_post);
    svr.Put("/", handle_post);
    svr.Post("/", handle_post);
    svr.Delete("/", handle_post);

    try {
        cout << "Opening server for requests on host " << host << " at port " << port << std::endl;
#ifdef CPPHTTPLIB_THREAD_POOL_COUNT
        cout << "Using " << CPPHTTPLIB_THREAD_POOL_COUNT << " threads." << std::endl;
#else
        cout << "Using " << std::thread::hardware_concurrency() << " threads." << std::endl;
#endif
        cout << "Press CTRL+C to exit." << std::endl;
        if (!svr.listen(host, port)) {
            std::cerr << "Server stopped in error state" << endl;
            return EXIT_FAILURE;
        }
    } catch (const std::exception& e) {
        cerr << "Hit an error." << endl;
        cerr << "Error message: " << e.what() << endl;
        return EXIT_FAILURE;
    } catch (...) {
        cerr << "Hit an error." << endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
