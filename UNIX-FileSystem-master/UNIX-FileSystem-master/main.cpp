#define MAIN
#include "head.h"
//命令处理所用的头文件
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

//存储所有打开文件
map<string, File*> file_open;

vector<string> split(string str)
{
	string buf;
	stringstream ss(str);
	vector<string> v;
	// 字符流ss 
	while (ss >> buf) {
		//转小写
		transform(buf.begin(), buf.end(), buf.begin(), ::tolower);
		v.push_back(buf);
	}

	return v;
}

void Switch_Demand(vector<string> demand_vector)
{
	if (demand_vector[0] == "help") {
		if (demand_vector.size() == 1) {
			help();
		}
		else if (demand_vector[1] == "attrib") {
			help_attrib();
		}
		else if (demand_vector[1] == "cd") {
			help_cd();
		}
		else if (demand_vector[1] == "del") {
			help_del();
		}
		else if (demand_vector[1] == "dir") {
			help_dir();
		}
		else if (demand_vector[1] == "exit") {
			help_exit();
		}
		else if (demand_vector[1] == "mkdir") {
			help_mkdir();
		}
		else if (demand_vector[1] == "rmdir") {
			help_rmdir();
		}
		else if (demand_vector[1] == "print") {
			help_print();
		}
		else if (demand_vector[1] == "write") {
			help_write();
		}
		else if (demand_vector[1] == "open") {
			help_open();
		}
		else if (demand_vector[1] == "close") {
			help_close();
		}
		else if (demand_vector[1] == "fseek") {
			help_fseek();
		}
		else if (demand_vector[1] == "logout") {
			help_logout();
		}
		else if (demand_vector[1] == "whoami") {
			help_whoami();
		}
		else if (demand_vector[1] == "format") {
			help_format();
		}
		else if (demand_vector[1] == "register") {
			help_register();
		}
		else if (demand_vector[1] == "deleteaccount") {
			help_deleteaccount();
		}
		else if (demand_vector[1] == "su") {
			help_su();
		}
		else if (demand_vector[1] == "chgrp") {
			help_chgrp();
		}
		else if (demand_vector[1] == "userlist") {
			help_userlist();
		}
		else if (demand_vector[1] == "create") {
			help_create();
		}
		else if (demand_vector[1] == "openlist") {
			help_openlist();
		}
		else {
			cout << "命令不存在" << endl;
		}

	}
	else if (demand_vector[0] == "attrib") {
		if (demand_vector.size() != 4)
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else if (demand_vector[1].size() != 2)
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else if (demand_vector[1][0] != '+' && demand_vector[1][0] != '-')
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else if (demand_vector[1][1] != 'r' && demand_vector[1][1] != 'w' && demand_vector[1][1] != 'e')
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else if (demand_vector[2].size() != 1)
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else if (demand_vector[2][0] != 'o' && demand_vector[2][0] != 'g' && demand_vector[2][0] != 'e')
			cout << "ATTRIB：命令语法不正确，使用\"HELP ATTRIB\"命令查看使用规则" << endl;
		else {
			unsigned int permission = 0;
			bool add;
			if (demand_vector[1][0] == '+')
				add = true;
			else
				add = false;
			if (demand_vector[1][1] == 'r' && demand_vector[2] == "o")
				permission = Inode::OWNER_R;
			else if (demand_vector[1][1] == 'w' && demand_vector[2] == "o")
				permission = Inode::OWNER_W;
			else if (demand_vector[1][1] == 'e' && demand_vector[2] == "o")
				permission = Inode::OWNER_E;
			else if (demand_vector[1][1] == 'r' && demand_vector[2] == "g")
				permission = Inode::GROUP_R;
			else if (demand_vector[1][1] == 'w' && demand_vector[2] == "g")
				permission = Inode::GROUP_W;
			else if (demand_vector[1][1] == 'e' && demand_vector[2] == "g")
				permission = Inode::GROUP_E;
			else if (demand_vector[1][1] == 'r' && demand_vector[2] == "e")
				permission = Inode::ELSE_R;
			else if (demand_vector[1][1] == 'w' && demand_vector[2] == "e")
				permission = Inode::ELSE_W;
			else if (demand_vector[1][1] == 'e' && demand_vector[2] == "e")
				permission = Inode::ELSE_E;
			Edit_File_Permission(demand_vector[3].c_str(), permission, add);
			cout << "已成功更改文件读写属性" << endl;
		}
	}
	else if (demand_vector[0] == "cd") {
		if (demand_vector.size() == 1) {
			cout << Current_Directory() << endl;
		}
		else {
			Open_Directory(demand_vector[1].c_str());
		}
	}
	else if (demand_vector[0] == "del") {
		if (demand_vector.size() < 2)
			cout << "DEL：命令语法不正确，使用\"HELP DEL\"命令查看使用规则" << endl;
		else {
			for (int i = 1; i < demand_vector.size(); i++) {
				Delete_File(demand_vector[i].c_str());
				cout << "成功删除文件" << demand_vector[i] << endl;
			}
		}
	}
	else if (demand_vector[0] == "dir") {
		if (demand_vector.size() > 2)
			cout << "DIR：命令语法不正确，使用\"HELP DIR\"命令查看使用规则" << endl;
		else if (demand_vector.size() == 2 && demand_vector[1] != "/q")
			cout << "DIR：命令语法不正确，使用\"HELP DIR\"命令查看使用规则" << endl;
		else {
			if (demand_vector.size() == 1)
				Show_File_List(false);
			else
				Show_File_List(true);
		}
	}
	else if (demand_vector[0] == "exit") {
		if (demand_vector.size() > 2)
			cout << "EXIT：命令语法不正确，使用\"HELP EXIT\"命令查看使用规则" << endl;
		else if (demand_vector.size() == 2 && !isdigit(demand_vector[1][0]))
			cout << "EXIT：命令语法不正确，使用\"HELP EXIT\"命令查看使用规则" << endl;
		else {
			if (demand_vector.size() == 1) {
				cout << "退出文件系统" << endl;
				exit(0);
			}
			else {
				cout << "退出文件系统" << endl;
				exit((int)demand_vector[1][0] - 48);
			}
		}
	}
	else if (demand_vector[0] == "mkdir") {
		if (demand_vector.size() != 2)
			cout << "MKDIR：命令语法不正确，使用\"HELP MKDIR\"命令查看使用规则" << endl;
		else {
			Create_Directory(demand_vector[1].c_str());
			cout << "成功创建目录" << demand_vector[1] << endl;
		}
	}
	else if (demand_vector[0] == "rmdir") {
		if (demand_vector.size() != 2)
			cout << "MKDIR：命令语法不正确，使用\"HELP RMDIR\"命令查看使用规则" << endl;
		else {
			Remove_Directory(demand_vector[1].c_str());
			cout << "成功删除目录" << demand_vector[1] << endl;
		}
	}
	else if (demand_vector[0] == "open") {
		if (demand_vector.size() != 2)
			cout << "OPEN：命令语法不正确，使用\"HELP OPEN\"命令查看使用规则" << endl;
		else {
			File* file = Open_File(demand_vector[1].c_str());
			if (file_open.find(Current_Directory() + "\\" + demand_vector[1]) == file_open.end()) {
				file_open[Current_Directory() + "\\" + demand_vector[1]] = file;
				cout << "已成功打开文件" << demand_vector[1] << endl;
			}
			else
			{
				cout << "当前文件已被打开，无法重复打开" << endl;
			}
		}
	}
	else if (demand_vector[0] == "close") {
		if (demand_vector.size() != 2)
			cout << "CLOSE：命令语法不正确，使用\"HELP CLOSE\"命令查看使用规则" << endl;
		else {
			auto it = file_open.find(Current_Directory() + "\\" + demand_vector[1]);
			if (it == file_open.end()) {
				cout << "未打开文件" << demand_vector[1] << endl;
			}
			else {
				Close_File(it->second);
				file_open.erase(it);
				cout << "已成功关闭文件" << demand_vector[1] << endl;
			}

		}
	}
	else if (demand_vector[0] == "print") {
		if (demand_vector.size() > 6)
			cout << "PRINT：命令语法不正确，使用\"HELP PRINT\"命令查看使用规则" << endl;
		else {
			int length = -1;
			vector<string>::iterator it;
			for (it = demand_vector.begin(); it != demand_vector.end();it++) {
				if (*it == "-l")
					break;
				if (it->at(0) == '-' && !(it->at(1) == 'p' || it->at(1) == 'l')) {
					cout << "PRINT：命令语法不正确，使用\"HELP PRINT\"命令查看使用规则" << endl;
					return;
				}
			}
			if (it != demand_vector.end())
			{
				it++;
				if (!isdigit(it->at(0))) {
					cout << "PRINT：命令语法不正确，使用\"HELP PRINT\"命令查看使用规则" << endl;
					return;
				}
				length = atoi((*it).c_str());
			}

			char* content = new char[100000000];
			auto i = file_open.find(Current_Directory() + "\\" + demand_vector[1]);
			if (i == file_open.end()) {
				cout << "未打开文件" << demand_vector[1] << endl;
				return;
			}
			Read_File(i->second, content,length);
			

			for (it = demand_vector.begin(); it != demand_vector.end(); it++) {
				if (*it == "-p")
					break;
			}
			if (it != demand_vector.end())
			{
				it++;
				fstream f(*it, ios::out);
				if (!f.is_open()) {
					cout << "无法写入文件" << *it << endl;
					return;
				}
				f << content;
				f.close();
				cout << "已将" << demand_vector[1] << "文件内容写入" << *it << endl;
			}
			else {
				cout << content << endl;
			}
			delete[]content;
		}
	}
	else if (demand_vector[0] == "write") {
		if (demand_vector.size() > 4)
			cout << "WRITE：命令语法不正确，使用\"HELP WRITE\"命令查看使用规则" << endl;
		else if (demand_vector[2] != "-s" && demand_vector[2] != "-f")
			cout << "WRITE：命令语法不正确，使用\"HELP WRITE\"命令查看使用规则" << endl;

		else {
			auto it = file_open.find(Current_Directory() + "\\" + demand_vector[1]);
			if (it == file_open.end()) {
				cout << "未打开文件" << demand_vector[1] << endl;
				return;
			}

			if (demand_vector[2] == "-s") {
				Write_File(it->second, demand_vector[3].c_str());
				cout << "成功写入文件" << endl;
			}
			else {
				string content;
				fstream f(demand_vector[3], ios::in);
				if (!f.is_open()) {
					cout << "无法打开文件" << demand_vector[2] << endl;
					return;
				}
				while (!f.eof()) {
					getline(f, content);
					content += '\n';
					Write_File(it->second, content.c_str());
				}
				f.close();
				cout << "已将" << demand_vector[3] << "文件内容写入" << demand_vector[1] << endl;
			}
		}
	}
	else if (demand_vector[0] == "fseek") {
		if (demand_vector.size() != 3)
			cout << "FSEEK：命令语法不正确，使用\"HELP FSEEK\"命令查看使用规则" << endl;
		else {
			auto it = file_open.find(Current_Directory() + "\\" + demand_vector[1]);
			if (it == file_open.end()) {
				cout << "未打开文件" << demand_vector[1] << endl;
			}
			else {
				Seek_File(it->second, atoi(demand_vector[2].c_str()));
				cout << "已将" << demand_vector[1] << "文件指针定位到" << demand_vector[2] << endl;
			}
		}
	}
	else if (demand_vector[0] == "create") {
		if (demand_vector.size() != 2)
			cout << "CREATE：命令语法不正确，使用\"HELP CREATE\"命令查看使用规则" << endl;
		else {
			Create_File(demand_vector[1].c_str());
			cout << "成功创建文件" << demand_vector[1] << endl;
		}
	}
	else if (demand_vector[0] == "logout") {
		if (demand_vector.size() != 1)
			cout << "LOGOUT：命令语法不正确，使用\"HELP LOGOUT\"命令查看使用规则" << endl;
		else {
			User_Logout();
			cout << "用户成功登出" << endl;
		}
	}
	else if (demand_vector[0] == "whoami") {
		if (demand_vector.size() != 1)
			cout << "WHIAMI：命令语法不正确，使用\"HELP WHIAMI\"命令查看使用规则" << endl;
		else {
			char user_name[USER_NAME_MAX];
			cout << "userid：" << Get_User(user_name) << endl;
			cout << "username：" << user_name << endl;
		}
	}
	else if (demand_vector[0] == "openlist") {
		if (demand_vector.size() != 1)
			cout << "WHIAMI：命令语法不正确，使用\"HELP WHIAMI\"命令查看使用规则" << endl;
		else {
			cout << "当前打开的文件有：" << endl;
			for (auto& it : file_open) {
				cout << it.first << endl;
			}
		}
	}
	else if (demand_vector[0] == "format") {
		if (demand_vector.size() != 1)
			cout << "FORMAT：命令语法不正确，使用\"HELP FORMAT\"命令查看使用规则" << endl;
		else {
			Init();
			Activate();
			cout << "已成功格式化文件卷" << endl;
		}
	}
	else if (demand_vector[0] == "register") {
		if (demand_vector.size() != 3)
			cout << "REGISTER：命令语法不正确，使用\"HELP REGISTER\"命令查看使用规则" << endl;
		else {
			User_Register(demand_vector[1].c_str(), demand_vector[2].c_str());
			cout << "成功创建用户" << endl;
		}
	}
	else if (demand_vector[0] == "deleteaccount") {
		if (demand_vector.size() != 2)
			cout << "DELETEACCOUNT：命令语法不正确，使用\"HELP DELETEACCOUNT\"命令查看使用规则" << endl;
		else {
			User_Delete(demand_vector[1].c_str());
			cout << "成功删除用户" << endl;
		}
	}
	else if (demand_vector[0] == "su") {
		if (demand_vector.size() != 3)
			cout << "SU：命令语法不正确，使用\"HELP SU\"命令查看使用规则" << endl;
		else {
			User_Logout();
			User_Login(demand_vector[1].c_str(), demand_vector[2].c_str());
			cout << "成功更换用户" << endl;
		}
	}
	else if (demand_vector[0] == "chgrp") {
		if (demand_vector.size() != 3)
			cout << "CHGRP：命令语法不正确，使用\"HELP CHGRP\"命令查看使用规则" << endl;
		else {
			Change_User_Group(demand_vector[1].c_str(), atoi(demand_vector[2].c_str()));
			cout << "成功更改用户" << demand_vector[1]<<"到"<< demand_vector[2]<<"组"<< endl;
		}
	}
	else if (demand_vector[0] == "userlist") {
		if (demand_vector.size() != 1)
			cout << "USERLIST：命令语法不正确，使用\"HELP USERLIST\"命令查看使用规则" << endl;
		else {
			Show_User_List();
		}
	}
	else {
		cout << demand_vector[0] << "不是内部或外部命令，也不是可运行的程序或批处理文件。" << endl;
	}

}



int main()
{
	cout << "===============================================" << endl;
	cout << "              仿UNIX V6++文件系统              " << endl;
	cout << "                 鞠璇  1851846                 " << endl;
	cout << "===============================================" << endl;
	cout << endl;

	help();

	fstream fd;
	fd.open(DISK_NAME, ios::in);
	if (!fd.is_open()) {
		cout << "文件系统不存在，按任意键进行初始化..." << endl;
		while (getchar() != '\n');
		Init();
	}
	else {
		fd.close();
		cout << "是否进行文件系统初始化？（y/n）" << endl;
		char ch;
		if ((ch = getchar()) == 'y') {
			Init();
		}
		while (ch != '\n' && getchar() != '\n');
	}

	Activate();


	string demand;
	while (1) {
		try {
			if (Get_User(NULL) == unsigned short(-1)) {
				cout << "用户未登录，请输入用户名和密码" << endl;
				cout << "用户名： ";
				char user_name[USER_NAME_MAX];
				cin >> user_name;
				cout << "密码： ";
				char user_password[USER_PASSWORD_MAX];
				cin >> user_password;
				User_Login(user_name, user_password);
				getchar();
			}
			cout << Current_Directory() << ">";
			getline(cin, demand);
			vector<string> demand_vector = split(demand);

			if (demand_vector.empty())
				continue;
			else {
				Switch_Demand(demand_vector);
			}
		}
		catch (int& e) {
			cout << "【错误码】" << e << endl;
		}
		cout << endl;
	}


	return 0;
}