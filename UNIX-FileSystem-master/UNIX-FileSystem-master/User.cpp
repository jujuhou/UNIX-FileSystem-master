#include "head.h"
#include "error.h"


//登录用户
void User_Login(const char* user_name, const char* password)
{
	User user;
	Read_User(user);
	bool find_user=false;
	for (int i = 0; i < USER_NUM; i++) {
		if (strcmp(user.u_name[i], user_name) == 0) {
			find_user = true;
			if (strcmp(user.u_password[i], password) == 0) {
				user_id = i;
			}
			else {
				cout << "密码错误" << endl;
				throw(ERROR_PASSWORD_WRONG);
			}
			
		}
	}
	if (find_user == false) {
		cout << "此用户不存在" << endl;
		throw(ERROR_USER_NOT_EXIST);

	}
}

//登出用户
void User_Logout()
{
	user_id = -1;
}

//创建用户
void User_Register(const char* user_name, const char* password)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "只有root用户可以进行注册" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	int i;
	for (i = 0; i < USER_NUM; i++) {
		if (strcmp(user.u_name[i], user_name) == 0) {
			cout << "用户名已存在，无法注册" << endl;
			throw(ERROR_USER_REGISTER);
		}
	}
	for (i = 0; i < USER_NUM; i++) {
		if (strlen(user.u_name[i])==0) {
			strcpy(user.u_name[i], user_name);
			strcpy(user.u_password[i], password);
			user.u_id[i] = i;
			user.u_gid[i] = 2;
			break;
		}
	}
	if (i == USER_NUM) {
		cout << "用户量已达上限制，无法继续注册" << endl;
		throw(ERROR_USER_REGISTER);
	}
	Write_User(user);
}

//删除用户
void User_Delete(const char* user_name)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "只有root用户可以进行用户删除" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	bool find_user = false;
	for (int i = 0; i < USER_NUM; i++) {
		if (strcmp(user.u_name[i], user_name) == 0) {
			find_user = true;
			user.u_name[i][0] = '\0';
			user.u_password[i][0] = '\0';
		}
	}
	
	if (find_user == false) {
		cout << "此用户不存在" << endl;
		throw(ERROR_USER_NOT_EXIST);
	}

	Write_User(user);
}
//获取当前登录的用户的用户名和用户id
unsigned int Get_User(char* username)
{
	if (username != NULL) {
		User user;
		Read_User(user);
		strcpy(username, user.u_name[user_id]);
	}
	return user_id;
}
//更改用户的所属的组
void Change_User_Group(const char* user_name,unsigned int user_group)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "只有root用户可以进行用户组更改" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	bool find_user = false;
	int i;
	for (i = 0; i < USER_NUM; i++) {
		if (strcmp(user.u_name[i], user_name) == 0) {
			find_user = true;
			break;
		}
	}

	if (find_user == false) {
		cout << "此用户不存在" << endl;
		throw(ERROR_USER_NOT_EXIST);
	}
	user.u_gid[i] = user_group;
	Write_User(user);
}
//显示用户列表
void Show_User_List()
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "只有root用户可以查看用户列表" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	unsigned int username_length = 20,
		userid_length = 15,
		usergroupid_length = 15;

	cout << setw(username_length) << "UserName"
		<< setw(userid_length) << "UserId"
		<< setw(usergroupid_length) << "UserGroupId" << endl;

	for (int i = 0; i < USER_NUM; i++) {
		if (strlen(user.u_name[i])) {
			cout << setw(username_length) << user.u_name[i]
				<< setw(userid_length) << user.u_id[i]
				<< setw(usergroupid_length) << user.u_gid[i] << endl;
		}
	}

}