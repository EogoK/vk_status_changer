#ifndef SPOTIFY_H
#define SPOTIFY_H
#include "l.h"
#include "Header.h"

class Spotify {
public:

	explicit Spotify(string client_id, string client_sec, HINTERNET& hinter, string code, string base) :cid{ client_id }, cs{ client_sec }, open{ hinter }, code_s{ code }, base_cscid{base}{}


	wstring get_music_spotify_playing() {
		
		auto connect = InternetConnectA(open, "api.spotify.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
		auto httpOpenReq = HttpOpenRequestA(connect, "GET", "v1/me/player", NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);
		
		string header = "Authorization: " + api_key_spotify;
		HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);
		header = "Accept-Language: ru-RU,ru;q=0.9,en;q=0.8";
		HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);
		header = "Cache-Control: max-age=0";
		HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);

		auto httpSendReq = HttpSendRequest(httpOpenReq, NULL, 0, NULL, 0);
		if (httpSendReq) {
			unique_ptr<char> data(new char[4096]);
			auto ptr = data.get();
			ZeroMemory(ptr, 4096);
			string str, temp;
			DWORD bytesRead = 1;
			bool log = 1;
			while (bytesRead && log) {
				ZeroMemory(ptr, 4096);
				temp.clear();
				log = InternetReadFile(httpOpenReq, ptr, 4096, &bytesRead);
				temp.append(ptr);
				str.append(temp.substr(0, bytesRead));
			}
			str = clear_space_special(str);

			wstring name, art, full = L"";
			auto json = string_to_json(str);
			if (json.size()) {
				if (json[0].first == "error") {
					InternetCloseHandle(httpOpenReq);
					InternetCloseHandle(connect);
					return L"1";
				}
				json = string_to_json(json[6].second);
				if (json.size() >= 11) {
					name = stw(json[11].second);
					name = name.substr(1, name.size() - 2);
					json = string_to_json(json[1].second.substr(1, json[1].second.size() - 2));
					art = stw(json[3].second);
					art = art.substr(1, art.size() - 2);
					full = art + L" - " + name + L"(Spotify)";
				}
				InternetCloseHandle(httpOpenReq);
				InternetCloseHandle(connect);
				return full;
			}
		}
		InternetCloseHandle(httpOpenReq);
		InternetCloseHandle(connect);
		return L"";
	}

	void update_key() {
	    
		string opt = "grant_type=refresh_token&refresh_token=" + refresh_token;

		auto connect = InternetConnectA(open, "accounts.spotify.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
		auto httpOpenReq = HttpOpenRequestA(connect, "POST", "api/token", NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);

		string header = "Content-Type:application/x-www-form-urlencoded";
		HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);
		header = "Authorization: Basic " + base_cscid;
		HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);

		auto httpSendReq = HttpSendRequest(httpOpenReq, NULL, 0, (LPVOID)opt.c_str(), opt.size());
		if (httpSendReq) {
			unique_ptr<char> data(new char[1024]);
			auto ptr = data.get();
			ZeroMemory(ptr, 1024);
			string str;
			DWORD bytesRead = 1;
			bool log = 1;
			while (bytesRead && log) {
				log = InternetReadFile(httpOpenReq, ptr, 1024, &bytesRead);
				str.append(ptr);
				ZeroMemory(ptr, 1024);
			}
			auto json = string_to_json(str);
			api_key_spotify = json[1].second.substr(1, json[1].second.size() - 2) + " " + json[0].second.substr(1, json[0].second.size() - 2);
		}
		InternetCloseHandle(httpOpenReq);
		InternetCloseHandle(connect);
	}
	string authorize_key() {
		get_token_refresh();
		if (refresh_token.empty()) {
			string opt = "grant_type=authorization_code&code=" + code_s + "&redirect_uri=https%3A%2F%2Fdruzhochkov.000webhostapp.com%2Fcallback%2F";
			auto connect = InternetConnectA(open, "accounts.spotify.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
			auto httpOpenReq = HttpOpenRequestA(connect, "POST", "api/token", NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);

			string header = "Content-Type:application/x-www-form-urlencoded";
			HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);
			header = "Authorization: Basic " + base_cscid;
			HttpAddRequestHeadersA(httpOpenReq, header.c_str(), header.length(), HTTP_ADDREQ_FLAG_ADD);

			auto httpSendReq = HttpSendRequest(httpOpenReq, NULL, 0, (LPVOID)opt.c_str(), opt.size());
			if (httpSendReq) {
				unique_ptr<char> data(new char[1024]);
				auto ptr = data.get();
				ZeroMemory(ptr, 1024);
				string str;
				DWORD bytesRead = 1;
				bool log = 1;
				while (bytesRead && log) {
					log = InternetReadFile(httpOpenReq, ptr, 1024, &bytesRead);
					str.append(ptr);
					ZeroMemory(ptr, 1024);
				}
				auto json = string_to_json(str);
				
				api_key_spotify = json[1].second.substr(1, json[1].second.size() - 2) + " " + json[0].second.substr(1, json[0].second.size() - 2);
				refresh_token = json[3].second.substr(1, json[3].second.size() - 2);
				save_token_refresh();
				return api_key_spotify;
			}
			InternetCloseHandle(httpOpenReq);
			InternetCloseHandle(connect);
			return "error1";
		}
		else {
			update_key();
		}
		return "error2";
	}
	
	void get_token_refresh(){
		ifstream file("spotify.dat");
		file >> refresh_token;
		file.close();
	}

	void save_token_refresh() const {
		if (!refresh_token.empty()) {
			ofstream file("spotify.dat");
			file << refresh_token;
			file.close();
		}
		return;
	}
private:
	HINTERNET open;

	string cid;
	string cs;
	string base_cscid;
	string refresh_token;
	string api_key_spotify;
	string code_s;

	string get_artist(string data) {
		
	}
};

#endif