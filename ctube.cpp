#include <algorithm>
#include <cstddef>
#include <curl/curl.h>
#include <fstream>
#include <ios>
#include <iterator>
#include <string>
#include <iostream>
#include <vector>
#include "json.hpp"
//#include "convertor.hpp"

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
        output.erase(0, pos_begin + first_sub.size());// if found, delete
    else
        std::cerr << "Error: first sub is not found" << std::endl;


    size_t pos_end = output.find(last_sub);
    if (pos_end != std::string::npos)
        output.erase(pos_end);
    else
        std::cerr << "Error: last sub not found" << std::endl;

}

void get_download_url(std::string& video_url){
    delete_unnessecary_output(video_url, "\"url\": \"", "\"");
}

std::string extract_videoId(const char* link){
    std::string videoId = link;
    delete_unnessecary_output(videoId, "https://youtu.be/", "`");
    delete_unnessecary_output(videoId, "watch?v=", "?si=");
    return videoId;
}

std::vector<std::string> get_audio_download_url(nlohmann::json& data, size_t adaptiveFormats_size){
    std::vector<std::string> audio_urls;
    for (size_t i = 0; i < adaptiveFormats_size; i++){
        std::string mimeType = data["streamingData"]["adaptiveFormats"][i]["mimeType"];
        size_t pos = mimeType.find("audio/webm");
        if (pos == std::string::npos)
            continue;
        std::string audio_url = data["streamingData"]["adaptiveFormats"][i]["url"];
        audio_urls.push_back(audio_url);
    }
    return audio_urls;
}


int main(int argc, char *argv[]) {
    CURL* curl = curl_easy_init();
    std::string response;

    std::string body = R"({
      "context": {
        "client": {
          "hl": "en",
          "gl": "US",
          "clientName": "ANDROID",
          "clientVersion": "21.02.35",
          "clientScreen": "WATCH",
          "androidSdkVersion": 30,
          "userAgent": "com.google.android.youtube/21.02.35 (Linux; U; Android 11) gzip",
          "osName": "Android",
          "osVersion": "11"
        },
        "thirdParty": {
          "embedUrl": "https://www.youtube.com/"
        }
      },
      "playbackContext": {
        "contentPlaybackContext": {
          "signatureTimestamp": 19250
        }
      },
      "racyCheckOk": true,
      "contentCheckOk": true,
    )";

    std::string body_tv = R"({
        "context": {
            "client": {
                "hl": "en",
                "gl": "US",
                "clientName": "ANDROID_MUSIC",
                "clientVersion": "5.26.1",
                "clientScreen": "WATCH",
                "androidSdkVersion": 31
            }
        },
        "racyCheckOk": true,
        "contentCheckOk": true,
    )";

    std::string videoId = extract_videoId(argv[1]);
    body += "  \"videoId\": \"";
    body += videoId;
    body += "\"\n}";


    std::cout << body << std::endl;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: com.google.android.apps.youtube.vr.oculus/1.71.26 (Linux; U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip");
    headers = curl_slist_append(headers, "X-YouTube-Client-Name: 28");
    headers = curl_slist_append(headers, "X-YouTube-Client-Version: 1.71.26");

    curl_easy_setopt(curl, CURLOPT_URL, "https://www.youtube.com/youtubei/v1/player");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_variable);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;

    std::string video_url = response;
    get_download_url(video_url);

    curl_slist_free_all(headers);

    curl_easy_reset(curl);
    

    std::ofstream video("video.mp4", std::ios::binary);

    struct curl_slist* download_headers = nullptr;
    download_headers = curl_slist_append(download_headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 Chrome/120.0.0.0 Safari/537.36");
    download_headers = curl_slist_append(download_headers, "Referer: https://www.youtube.com/");

    curl_easy_setopt(curl, CURLOPT_URL, video_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &video);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, download_headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects

    res = curl_easy_perform(curl);

    // check what actually happened
    long httpCode = 0;
    if (res != CURLE_OK)
        std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;

    curl_slist_free_all(download_headers);


    video.close();

    std::ofstream file("url.txt");
    file << response;
    file.close();

    //std::ifstream file("url.txt");
    //
    curl_easy_reset(curl);

    nlohmann::json data = nlohmann::json::parse(response);

    //std::cout << data["streamingData"]["adaptiveFormats"].size() << std::endl;
    std::vector<std::string> audio_urls = std::move(get_audio_download_url(data, data["streamingData"]["adaptiveFormats"].size()));

    std::ofstream audio_file("audio.mp3", std::ios::binary);
    for (auto& url : audio_urls){
        struct curl_slist* audio_headers = nullptr;
        audio_headers = curl_slist_append(audio_headers, "Content-Type: application/json");
        audio_headers = curl_slist_append(audio_headers, "User-Agent: com.google.android.apps.youtube.vr.oculus/1.71.26 (Linux; U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip");
        audio_headers = curl_slist_append(audio_headers, "X-YouTube-Client-Name: 28");
        audio_headers = curl_slist_append(audio_headers, "X-YouTube-Client-Version: 1.71.26");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &audio_file);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, audio_headers);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects

        res = curl_easy_perform(curl);

        // check what actually happened
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        if (res != CURLE_OK) {
            std::cerr << "curl error: " << curl_easy_strerror(res) << std::endl;
        } else if (httpCode != 200) {
            std::cerr << "HTTP error: " << httpCode << std::endl;  // will print 403
        } else {
            std::cout << url << std::endl;
            std::cout << "Success!" << std::endl;
            curl_slist_free_all(audio_headers);
            break;
        }
        curl_slist_free_all(audio_headers);
    }


    audio_file.close();

    curl_easy_cleanup(curl);

    return 0;
    //У каждой стороні есть две медали кто-то зашифровует ссілки, а ктото находит рабочий клиент ютуба и ему похуя на зашифровку
}
