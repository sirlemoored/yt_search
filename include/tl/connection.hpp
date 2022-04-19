#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

#include <nlohmann/json.hpp>

#include <tl/parameters.hpp>

#include <iostream>
#include <list>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;
using     json  = nlohmann::json;

namespace tl
{
    
    const std::string API_HOST                      = "www.googleapis.com";
    const std::string API_PORT                      = "443";
    const std::string API_DEFAULT_TARGET            = "/youtube/v3";
    const std::string API_CHANNELS_TARGET           = "/youtube/v3/channels";
    const std::string API_PLAYLIST_ITEMS_TARGET     = "/youtube/v3/playlistItems";
    const std::string API_COMMENT_THREADS_TARGET    = "/youtube/v3/commentThreads";
    const int         HTTP_VERSION                  = 11;

    class connection
    {
        private:

            asio::io_context&                       io_context;
            asio::ssl::context                      ssl_context;
            beast::ssl_stream<beast::tcp_stream>    stream;
            beast::error_code                       error;
            std::string                             api_key;

        private:

            std::string perform_http_request_(http::verb method, 
                                              const std::string& host,
                                              const std::string& target,
                                              int http_version)
            {
                http::request<http::dynamic_body> request;
                request.method(method);
                request.target(target);
                request.version(http_version);
                request.set(http::field::host, host);
                request.set(http::field::connection, "keep-alive");
                http::write(stream, request, error);


                if (error)
                    return "";

                beast::flat_buffer                  response_buffer;
                http::response<http::dynamic_body>  response;
                http::read(stream, response_buffer, response, error);

                
                if (error || response.result() != http::status::ok)
                    return "";
                else
                    return beast::buffers_to_string(response.body().data());
            }

        public:

            connection(asio::io_context& ioc, const std::string& key) : 
                io_context{ioc},
                ssl_context{asio::ssl::context::tlsv13_client},
                stream{io_context, ssl_context},
                api_key{key}
            {

            }

            void connect()
            {
                asio::ip::tcp::resolver resolver{io_context};
                
                auto results = resolver.resolve(API_HOST, API_PORT, error);
                beast::get_lowest_layer(stream).connect(results, error);
                stream.handshake(asio::ssl::stream_base::client, error);
            }

            void disconnect()
            {
                stream.shutdown(error);
                if (error == asio::error::eof)
                    error = {};
            }

            std::string get_channel_uploads_playlist(const std::string& channel_id)
            {
                tl::parameters query_params;
                query_params.target(API_CHANNELS_TARGET);
                query_params.set("key", api_key);
                query_params.set("id", channel_id);
                query_params.set("part", "contentDetails");

                std::string result = perform_http_request_(http::verb::get, API_HOST, query_params.to_string(), HTTP_VERSION);
                json j = json::parse(result);
                return j["items"][0]["contentDetails"]["relatedPlaylists"]["uploads"];
            }

            std::list<std::string> get_videos_from_playlist(const std::string& playlist_id)
            {
                std::string page_token;
                tl::parameters query_params;
                std::list<std::string> ids;
                do
                {
                    query_params.clear();
                    query_params.target(API_PLAYLIST_ITEMS_TARGET);
                    query_params.set("key", api_key);
                    query_params.set("playlistId", playlist_id);
                    query_params.set("part", "snippet");
                    query_params.set("maxResults", "50");
                    if (!page_token.empty())
                        query_params.set("pageToken", page_token);

                    std::string result = perform_http_request_(http::verb::get, API_HOST, query_params.to_string(), HTTP_VERSION);
                    json j = json::parse(result);

                    page_token = j.contains("nextPageToken") ? j["nextPageToken"] : "";

                    for (auto& item : j["items"])
                        ids.push_back(item["snippet"]["resourceId"]["videoId"]);

                }
                while (!page_token.empty());

                return ids;
            }

            std::list<std::string> get_comments_from_video(const std::string& video_id)
            {
                std::string page_token;
                tl::parameters query_params;
                std::list<std::string> comments;
                do
                {
                    query_params.clear();
                    query_params.target(API_COMMENT_THREADS_TARGET);
                    query_params.set("key", api_key);
                    query_params.set("videoId", video_id);
                    query_params.set("part", "snippet");
                    query_params.set("textFormat", "plainText");
                    query_params.set("maxResults", "100");
                    if (!page_token.empty())
                        query_params.set("pageToken", page_token);

                    std::string result = perform_http_request_(http::verb::get, API_HOST, query_params.to_string(), HTTP_VERSION);

                    if (!json::accept(result))
                        return comments;

                    json j = json::parse(result);

                    page_token = j.contains("nextPageToken") ? j["nextPageToken"] : "";
                    if (j.contains("items"))
                        for (auto& item : j["items"])
                        {
                            comments.push_back(item["snippet"]["topLevelComment"]["snippet"]["textDisplay"]);
                        }
                }
                while (!page_token.empty());

                return comments;
            }

    };

} // namespace tl


#endif