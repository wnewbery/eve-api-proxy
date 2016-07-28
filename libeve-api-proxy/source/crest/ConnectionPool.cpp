#include "Precompiled.hpp"
#include "ConnectionPool.hpp"
#include "Error.hpp"
#include "CacheOld.hpp"
#include "CppServers.hpp"
#include "Gzip.hpp"
#include "Log.hpp"
#include <iostream>
#include <chrono>
namespace crest
{
    namespace
    {
        http::AsyncClientParams client_params(http::SocketFactory *socket_factory)
        {
            http::AsyncClientParams ret;
            ret.tls = true;
            ret.host = PCREST_HOST;
            ret.port = PCREST_PORT;
            ret.max_connections = PCREST_MAX_CONNECTIONS;
            ret.rate_limit = PCREST_MAX_REQS_PER_SEC;
            ret.socket_factory = socket_factory;
            ret.default_headers.add("Accept-Encoding", "gzip");
            ret.default_headers.add("User-Agent", "eve-api-proxy");

            return ret;
        };
    }
    ConnectionPool::ConnectionPool()
        : socket_factory()
        , http_client(client_params(&socket_factory))
    {
    }

    ConnectionPool::~ConnectionPool()
    {
        exit();
    }

    void ConnectionPool::exit()
    {
        //TODO: Not implemented
    }

    void ConnectionPool::queue(http::AsyncRequest *request)
    {
        auto cb_tmp = request->on_completion;
        auto start = std::chrono::steady_clock::now();
        request->on_completion = [start, cb_tmp](http::AsyncRequest *request, http::Response &response)
        {
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            log_info()
                << elapsed.count() << "ms "
                << response.status.code << " " << response.status.msg
                << " " << request->raw_url << std::endl;
            if (cb_tmp) cb_tmp(request, response);
        };
        log_info() << "CREST GET " << request->raw_url << std::endl;
        http_client.make_request(request);
    }
}
