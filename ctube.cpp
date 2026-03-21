#include <cstddef>
#include <cstdio>
#include <curl/curl.h>
#include <fstream>
#include <ios>
#include <string>
#include <iostream>
#include <vector>
#include "clients.hpp"
// callback to collect response
size_t write_to_variable(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t write_to_file(void *data, size_t size, size_t nmemb, std::ofstream *userdata){
    size_t totalSize = size * nmemb;
    userdata->write((char*)data, totalSize);
    return totalSize;
}

void delete_unnessecary_output(std::string& output, const std::string& first_sub, const std::string& last_sub){
    size_t pos_begin = output.find(first_sub);
    if (pos_begin != std::string::npos)
        output.erase(0, pos_begin + first_sub.size());//if found, delete
    else
        std::cerr << "Error: first sub is not found" << std::endl;


    size_t pos_end = output.find(last_sub);
    if (pos_end != std::string::npos)
        output.erase(pos_end);
    else
        std::cerr << "Error: last sub not found" << std::endl;

}

void copy_part_string(const std::string& in_string, std::string& out_string, const std::string& first_sub, const std::string& last_sub){
    size_t pos_begin = in_string.find(first_sub);
    if (pos_begin == std::string::npos){
        std::cerr << "Error: first sub is not found" << std::endl;
        return;
    }
    pos_begin += first_sub.size();//we don't need finding sub in our string

    size_t pos_end = in_string.find(last_sub, pos_begin);
    if (pos_end == std::string::npos){
        std::cerr << "Error: last sub not found" << std::endl;
        return;
    }

    out_string = in_string.substr(pos_begin, pos_end-pos_begin);
}

std::string get_download_url(std::string& player_response){
    //delete_unnessecary_output(player_response, "\"url\": \"", "\"");
    std::string video_url;
    copy_part_string(player_response, video_url, "\"url\": \"", "\"");

    return video_url;
}

std::string get_video_title(std::string& player_response){
    std::string title;
    copy_part_string(player_response, title, "\"title\": \"", "\"");
    return title;
}

std::string extract_videoId(const char* link){
    std::string videoId = link;
    delete_unnessecary_output(videoId, "https://youtu.be/", "`");
    delete_unnessecary_output(videoId, "watch?v=", "?si=");
    return videoId;
}

std::string get_body_to_video(Clients& clients, const char* link){
    std::string body = clients.clients[0].second;
    //std::cout << link << std::endl;
    std::string videoId = extract_videoId(link);
    body += "  \"videoId\": \"";
    body += videoId;
    body += "\"\n}";
    return body;
}

void headers_to_player(curl_slist*& headers){
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: com.google.android.apps.youtube.vr.oculus/1.71.26 (Linux; U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip");
    headers = curl_slist_append(headers, "X-YouTube-Client-Name: 28");
    headers = curl_slist_append(headers, "X-YouTube-Client-Version: 1.71.26");
}

void make_ytplayer_request(CURL*& curl, std::string& body, curl_slist*& headers, CURLcode& res, std::string& response){
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.youtube.com/youtubei/v1/player");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_variable);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;

    curl_slist_free_all(headers);

    curl_easy_reset(curl);
}

void curl_download(CURL*& curl, CURLcode& res, std::string& url, std::string& file_name){
    std::ofstream video(file_name, std::ios::binary);


    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &video);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;


    video.close();

    curl_easy_reset(curl);
}

void convert_to_audio(std::string& video_name, std::string& audio_name){
    std::string ffmpeg_command = "ffmpeg -i '";
    ffmpeg_command += video_name;
    ffmpeg_command += "'";
    ffmpeg_command += " '";
    ffmpeg_command += audio_name;
    ffmpeg_command += "'";
    std::cout << ffmpeg_command << std::endl;
    system(ffmpeg_command.c_str());
}

void cleanup(CURL*& curl){
    curl_easy_cleanup(curl);
}

enum Options {smth, download_video_opt, download_audio_opt, download_playlist_opt, help};

void download_video(CURL*& curl, Clients& clients, std::string& link, std::string& video_title){
    std::string response;
    CURLcode res;

    std::string body = get_body_to_video(clients, link.c_str());

    struct curl_slist* headers = nullptr;
    headers_to_player(headers);

    make_ytplayer_request(curl, body, headers, res, response);

    std::ofstream file("url.txt");
    file << response;
    file.close();

    std::string video_url = get_download_url(response);

    video_title = get_video_title(response);
    std::string vid_file_name = video_title;
    vid_file_name += ".mp4";

    curl_download(curl, res, video_url, vid_file_name);
}

void download_audio(CURL*& curl, Clients& clients, std::string& link, std::string& video_title){
    std::string response;
    CURLcode res;

    download_video(curl, clients, link, video_title);

    std::string vid_file_name = video_title;
    vid_file_name += ".mp4";

    std::string aud_file_name = video_title;
    aud_file_name += ".mp3";

    convert_to_audio(vid_file_name, aud_file_name);
    remove(vid_file_name.c_str());
}

void process_command(Options option, CURL*& curl, Clients& clients){
    std::string link, video_title;
    switch (option) {
        case download_video_opt:
            std::cout << "Paste youtube link here:" << std::endl;
            std::cin >> link;
            download_video(curl, clients, link, video_title);
            break;
        case download_audio_opt:
            std::cout << "Paste youtube link here:" << std::endl;
            std::cin >> link;
            download_audio(curl, clients, link, video_title);
            break;
        default:
            std::cout << "Baka" << std::endl;
    }
}


int main(int argc, char *argv[]) {
    CURL* curl = curl_easy_init();
    CURLcode res;
    std::string response;

    Clients clients;

    int option;//generally, needs to be enum Option, but for now it's int because cin doesn't work on enum

    std::cout << "1. Download video\n" << "2. Download audio\n" << "3. Download playlist\n" << "4. Help\n";

    std::cin >> option;

    process_command((Options)option, curl, clients);

    /*std::string body = get_body_to_video(clients, argv[1]);

    struct curl_slist* headers = nullptr;
    headers_to_player(headers);

    make_ytplayer_request(curl, body, headers, res, response);

    std::ofstream file("url.txt");
    file << response;
    file.close();

    std::string video_url = get_download_url(response);

    std::string video_title = get_video_title(response);
    std::string vid_file_name = video_title;
    vid_file_name += ".mp4";

    curl_download(curl, res, video_url, video_title);


    std::string aud_file_name = video_title;

    aud_file_name += ".mp3";

    convert_to_audio(video_title, aud_file_name);*/


    cleanup(curl);

    return 0;
}
