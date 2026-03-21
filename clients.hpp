#include <vector>
#include <string>

struct Clients{
    std::vector<std::pair<std::string, std::string>> clients = {
      {"android", R"({
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
        )"},
        {"android_vr", R"({
          "context": {
          'client': {
                'clientName': 'ANDROID_VR',
                'clientVersion': '1.71.26',
                'deviceMake': 'Oculus',
                'deviceModel': 'Quest 3',
                'androidSdkVersion': 32,
                'userAgent': 'com.google.android.apps.youtube.vr.oculus/1.71.26 (Linux; U; Android 12L; eureka-user Build/SQ3A.220605.009.A1) gzip',
                'osName': 'Android',
                'osVersion': '12L',
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
        )"}
      };
};
