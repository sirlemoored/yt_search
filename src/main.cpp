#include <tl/connection.hpp>
#include <string>
#include <boost/algorithm/string.hpp>
#include <windows.h>

#pragma execution_character_set( "utf-8" )

int main(int argc, char** argv)
{   
    SetConsoleOutputCP( 65001 );

    const std::string channel_id = argv[1];
    const std::string key = argv[2];
    const std::string keyword = argv[3];


    asio::io_context io_context;
    tl::connection c{io_context, key};

    c.connect();
    std::string playlist_id             = c.get_channel_uploads_playlist(channel_id);
    std::list<std::string> video_ids    = c.get_videos_from_playlist(playlist_id);
    size_t count = 0;
    for (const std::string& id : video_ids)
    {
        std::list<std::string> comments = c.get_comments_from_video(id);
        for (const std::string& comm : comments)
        {
            std::string low = boost::algorithm::to_lower_copy(comm);
            if (low.find(keyword) != std::string::npos)
            {
                std::cout << "[ "<< id << " ] : " << comm << '\n';
                std::cout << "Processed " << count << " out of " << video_ids.size() 
                          << " (" << 100 * count / video_ids.size() << "%).\r";

            }

        }
        count++;
        std::cout << "Processed " << count << " out of " << video_ids.size() 
                << " (" << 100 * count / video_ids.size() << "%).\r";

    }

    c.disconnect();

    io_context.run();

    return 0;
}