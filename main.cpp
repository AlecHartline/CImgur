#include<iostream>
#include<curl/curl.h>
#include<string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "callback.h"

using namespace std;
using namespace rapidjson;

static bool DEBUG_MODE = true;


static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp){
		((string*)userp) -> append((char*) contents, size*nmemb);

		return size*nmemb;
}

int main(int argc, char** argv){
		
		if(argc < 4){
				cout << "Usage: <top|time> <day|week|month|year> <tags...>" << endl;
				exit(1);
		}

		CURL * curl;
		CURLcode res;
		string readBuffer;

		cout << "Initializing cURL" << endl;

		curl = curl_easy_init();

		if(curl == NULL){
				cout << "cURL failed initialization. Terminating process." << endl;
				exit(2);
		}

		cout << "cURL initialization successful." << endl;

		string sort = string(argv[1]);
		string time =  string(argv[2]);

		for(int i = 3; i < argc; ++i){
				
				string tag = string(argv[i]);
				string url = "https://api.imgur.com/3/gallery/t/"+tag+"/"+sort+"/"+time+"/1";	

				struct curl_slist * header = NULL;		
				header = curl_slist_append(header, "Authorization: Client-ID 7a5f6f0bea8a843");

				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
				curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

				res = curl_easy_perform(curl);
				if(res != CURLE_OK){
						cout << "Perform failed with code " << res << endl;
						exit(res);
				}
				if(DEBUG_MODE == true){
						cout << "Read: " << endl << readBuffer << endl;
				}

				Document d;
				d.Parse(readBuffer.c_str());

				cout << tag << ": " << endl;

				Value& items = d["data"]["items"];
				for(Value::ConstValueIterator itr = items.Begin(); itr != items.End(); ++itr){
						string title = string((*itr)["title"].GetString()).substr(0,20);
						if(title.length() < string((*itr)["title"].GetString()).length()){
								title += "...";
						}
						int views = (*itr)["views"].GetInt();
						string link = (*itr)["link"].GetString();

						printf("%30s [%-6i] (%s)\n", title.c_str(), views, link.c_str());
				}
				readBuffer = "";
				
		}
		curl_easy_cleanup(curl);
}
