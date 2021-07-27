#ifndef L_H
#define L_H
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <codecvt>
#include <vector>
#include <functional>
#include <algorithm>
#include <Windows.h>
#include <wininet.h>
#include <mutex>
#include <sstream>
#include <TlHelp32.h>
#include <thread>
#include <condition_variable>
#pragma warning(disable:4996)
#pragma comment(lib, "wininet.lib")
using namespace std;

vector<pair<string, string>> string_to_json(string str) {
	vector<string> block_raw;
	vector<pair<string, string>> m;
	block_raw.resize(1);
	int i = 0, j = 0;
	if (str[0] == '{' && str[str.size() - 1] == '}') {
		str = str.substr(1, str.size() - 2);
		for_each(str.begin(), str.end(), [&block_raw, &i, &j](auto& a) {
			a == '[' || a == '{' ? j++ : 0;
			a == ']' || a == '}' ? j-- : 0;
			if (j == 0) {
				NULL;
			}
			if (a == ',' && j == 0) {
				i++;
				block_raw.push_back("");
			}
			if(a != ',' || j > 0) block_raw[i].push_back(a);
			});

		for (auto& a : block_raw) {
			auto first = a.substr(0, a.find(':'));
			auto second = a.substr(a.find(':') + 1, a.size() - a.find(':') - 1);
			m.push_back({ first, second });
		}

		return move(m);
	}
	return {};
}

string html_converter(wstring str) {
	stringstream ss;
	wstring_convert<codecvt_utf8<wchar_t>> c;
	string s = c.to_bytes(str);
	ss << hex;
	for (char& a : s) {
		ss << "%" << int(static_cast<unsigned char>(a));
	}
	return ss.str();
}

wstring_convert<codecvt_utf8<wchar_t>> conv;
wstring stw(string str) {
	wstring str_2 = L"";
	if (str.size() > 2) {
		str_2 = conv.from_bytes(str);
		return move(str_2);
	}
	return move(str_2);
}


string clear_space_special(string str) {
	string temp;
	int j = 0;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\"') {
			if (j == 1) {
				j = 0;
			}
			else {
				j = 1;
			}
		}

		if (str[i] != ' ' || j == 1) {
			if(str[i] != '\n') temp.push_back(str[i]);
		}
	}
	return move(temp);
}

#endif