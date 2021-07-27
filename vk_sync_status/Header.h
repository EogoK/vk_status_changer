#ifndef HEADER_H
#define HEADER_H
#include "l.h"
#include "spotify.h"

class VkSync {
	int breaker = 0;
	condition_variable c;
public:
	mutex m;
	~VkSync() {
		unsync_set_status(noth);
	}

	pair<wstring, int> CheckerProgram(vector<pair<int, pair<wstring, int>>> table) {
		PROCESSENTRY32 prc;
		prc.dwSize = sizeof(PROCESSENTRY32);
		auto res = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		wstring str, str2;
		vector<pair<int, pair<wstring, int>>> allow;
		pair<int, pair<wstring, int>> ret = { 100, {L"", 0} };

		auto b = Process32First(res, &prc);
		while (b) {
			for_each(table.begin(), table.end(), [&allow, &prc, &str, &str2](auto& a) {
				str = wstring(prc.szExeFile);
				str2 = a.second.first.substr(0, a.second.first.find('|'));
				if (str2 == str) {
					allow.push_back(a);
					return;
				}
				});
			b = Process32Next(res, &prc);
		}

		for_each(allow.begin(), allow.end(), [&ret](auto& a) {
			if (ret.first > a.first) {
				ret = a;
			}
			});
		CloseHandle(res);
		return ret.second;
	}

	
	VkSync(wstring phone, wstring password, HINTERNET& opens) : phone_vk{ phone }, password_vk{ password }, open{opens}{

		get_api_key();
		if (api_key.empty()) {
			auto obj = wstring(L"/token?grant_type=password&client_id=3697615&client_secret=AlVXZFMUqyrnABp8ncuU&username=" + phone + L"&password=" + password + L"&v=5.130&2fa_supported=0");

			auto connect_v = InternetConnect(open, L"oauth.vk.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 1);
			auto httpOpenReq = HttpOpenRequest(connect_v, L"POST", obj.c_str(), NULL, NULL, 0, INTERNET_FLAG_SECURE, 1);
			auto httpSendReq = HttpSendRequest(httpOpenReq, NULL, 0, NULL, 0);
			if (httpSendReq) {
				char data[1024];
				string str;
				ZeroMemory(data, 1024);
				DWORD bytesRead = 1;
				bool log = 1;
				while (bytesRead && log) {
					log = InternetReadFile(httpOpenReq, data, 1023, &bytesRead);
					str.append(data);
					ZeroMemory(data, 1024);
				}
				auto json = string_to_json(str);
				for (auto& a : json) {
					if (a.first == "\"access_token\"") {
						api_key = wstring(a.second.begin(), a.second.end());
						api_key = api_key.substr(1, api_key.size() - 2);
						save_api_key();
						break;
					}
				}
			}
			InternetCloseHandle(httpOpenReq);
			InternetCloseHandle(connect_v);
		}
	}

	void ChangeStatusTo() {
		while (1) {
			{
				unique_lock<mutex>g(m);
				c.wait(g);
				if (breaker) break;
			}
			m.lock();
			auto connect = InternetConnect(open, L"api.vk.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
			string temp = html_converter(change);
			auto str_1 = wstring(template_string + api_key + L"&text=" + wstring(temp.begin(), temp.end()));
			auto httpOpenReq = HttpOpenRequest(connect, post.c_str(), str_1.c_str(), NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);
			auto httpSendReq = HttpSendRequest(httpOpenReq, header.c_str(), header.length(), NULL, 0);
			if (httpSendReq) {
				unique_ptr<char> data(new char [1024]);
				auto ptr = data.get();
				string str;
				ZeroMemory(ptr, 1024);
				DWORD bytesRead = 1;
				bool log = 1;
				while (bytesRead && log) {
					log = InternetReadFile(httpOpenReq, ptr, 1024, &bytesRead);
					str.append(ptr);
					ZeroMemory(ptr, 1024);
				}
				auto json = string_to_json(str);
			}
			m.unlock();
			InternetCloseHandle(httpOpenReq);
			InternetCloseHandle(connect);
		}
	}

	void Func_main(vector<pair<int, pair<wstring, int>>>& table, vector<wstring>& prefix, wstring& noth, Spotify& spoty) {
		spoty.authorize_key();
		wstring update = getStatus(0);
		wstring status;
		while (1) {
			if (breaker) break;
			auto important_prog = CheckerProgram(table);
			m.lock();
			auto spotify = spoty.get_music_spotify_playing();
			if (spotify == L"1") {
				spoty.authorize_key();
				spotify = L"";
			}

			m.unlock();
			if (important_prog.first != L"") {
				status = prefix[important_prog.second] + important_prog.first.substr(important_prog.first.find('|') + 1, important_prog.first.size() - 1);
			}
			if (spotify != L"") {
				status = spotify;
			}
			if (spotify.size() == 0 && important_prog.first.size() == 0) {
				status = noth;
			}
			if (update == status) {
				continue;
			}
			vk_status(status);
			update = status;
		}
	}

	void vk_status(wstring stat) { change = move(stat); c.notify_all(); };

	wstring getStatus(int sleep=5000){
		Sleep(sleep);
		wstring result = L"";
		m.lock();
		auto connect = InternetConnect(open, L"api.vk.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
		auto str_1 = wstring(L"/method/status.get?access_token=" + api_key + L"&v=5.130");
		auto httpOpenReq = HttpOpenRequest(connect, L"POST", wstring(str_1.begin(), str_1.end()).c_str(), NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);
		auto httpSendReq = HttpSendRequest(httpOpenReq, NULL, 0, NULL, 0);
		m.unlock();
		if (httpSendReq) {
			unique_ptr<char> data(new char[1024]);
			auto ptr = data.get();
			ZeroMemory(ptr, 1024);
			string str;
			DWORD bytesRead = 1;
			bool log = 1;
			m.lock();
			while (bytesRead && log) {
				log = InternetReadFile(httpOpenReq, ptr, 1024, &bytesRead);
				str.append(ptr);
				ZeroMemory(ptr, 1024);
			}
			m.unlock();
			auto json = string_to_json(str);
			auto json_1 = string_to_json(json[0].second);
			result = stw(json_1[0].second);
			if(result.size() > 0) result = result.substr(1, result.size() - 2);
			InternetCloseHandle(httpOpenReq);
			InternetCloseHandle(connect);
			return move(result);
		}
		InternetCloseHandle(httpOpenReq);
		InternetCloseHandle(connect);
		return move(result);
	}

	void break_all_thread() { breaker++; };


	void save_api_key() {
		if (api_key.size()) {
			ofstream file("data.dat");
			string str = string(api_key.begin(), api_key.end());
			file << str;
			file.close();
			return;
		}
		return;
	}
	void get_api_key() {
		try {
			ifstream file("data.dat");
			string str;
			file >> str;
			file.close();
			api_key = wstring(str.begin(), str.end());
		}
		catch (exception& ex) {
			cout << ex.what() << endl;
		}
		return;
	}

	wstring noth;

private:

	HINTERNET open;
	wstring phone_vk = {}, password_vk = {}, api_key = {}, begin_status = {}, change;
	wstring template_string = wstring(L"/method/status.set?v=5.130&access_token=");
	wstring header = wstring(L"Accept-Language: ru-RU,ru;q=0.9,en;q=0.8");
	wstring post = L"POST";


	void unsync_set_status(wstring str) {
		auto connect = InternetConnect(open, L"api.vk.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, NULL, 2);
		string temp = html_converter(change);
		auto str_1 = wstring(template_string + api_key + L"&text=" + str);
		auto httpOpenReq = HttpOpenRequest(connect, post.c_str(), str_1.c_str(), NULL, NULL, 0, INTERNET_FLAG_SECURE, 2);
		auto httpSendReq = HttpSendRequest(httpOpenReq, header.c_str(), header.length(), NULL, 0);
		if (httpSendReq) {
			unique_ptr<char> data(new char[1024]);
			auto ptr = data.get();
			string str;
			ZeroMemory(ptr, 1024);
			DWORD bytesRead = 1;
			bool log = 1;
			while (bytesRead && log) {
				log = InternetReadFile(httpOpenReq, ptr, 1024, &bytesRead);
				str.append(str);
				ZeroMemory(ptr, 1024);
			}
			auto json = string_to_json(str);
		}
		InternetCloseHandle(httpOpenReq);
		InternetCloseHandle(connect);
	}
};

#endif
