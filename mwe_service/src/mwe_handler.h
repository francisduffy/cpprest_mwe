#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>

namespace mwe {

class MweHandler
{
public:
    MweHandler(const utility::string_t& uri);

    pplx::task<void> open() { return listener_->open(); }
    pplx::task<void> close() { return listener_->close(); }

private:
    void handle_get(web::http::http_request message);
    void handle_put(web::http::http_request message);
    void handle_post(web::http::http_request message);
    void handle_delete(web::http::http_request message);

    std::shared_ptr<web::http::experimental::listener::http_listener> listener_;
    web::http::experimental::listener::http_listener_config config_;
};

}
