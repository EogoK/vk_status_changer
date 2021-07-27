#include "l.h"
#include "Header.h"
#include "spotify.h"

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "Russian");
	vector<wstring> prefix = { L"Играет в ", L"Работает в "};
	vector<pair<int, pair<wstring, int>>> table;
	table.push_back({ 1, {L"devenv.exe|Visual Studio", 1} });
	table.push_back({ 0, {L"cmd.exe|Cmd", 0} });
	table.push_back({ 1, {L"sublime_text.exe|Sublime Text", 1} });
	table.push_back({ 1, {L"Discord.exe|Discord", 0} });

	wstring UserAgent = L"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3112.113 Safari/537.36";
	HINTERNET open = InternetOpen(UserAgent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);


	wstring noth = L"Ничего не делает";
	wstring pass = L"", phone = L"";//login and pass vk
	string code = "";//if spotify dont authorize
	VkSync vk(phone, pass, open);
	Spotify spoty("", "", open, code, ""/*base64 client_id:client_secret*/);

	auto get_str = vk.getStatus(0);
	if (!get_str.empty()) noth = get_str;
	vk.noth = noth;

	HWND hwnd = GetConsoleWindow();
	HMENU menu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(menu, SC_CLOSE, MF_GRAYED);

	cout << "press \"F9\", if you will want to close program" << endl;
	
	thread thr(&VkSync::ChangeStatusTo, &vk);
	thread thr2(&VkSync::Func_main, &vk, ref(table), ref(prefix), ref(noth), ref(spoty));
	while (1) {
		if (GetKeyState(VK_F9) & 0x8000) {
			vk.break_all_thread();
			vk.vk_status(L"");
			break;
		}
	}
	
	if(thr.joinable()) thr.join();
	if(thr2.joinable()) thr2.join();

	EnableMenuItem(menu, SC_CLOSE, MF_ENABLED);
	InternetCloseHandle(open);
	return 0;
}