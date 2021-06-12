#include "head.h"
#include "error.h"


//��¼�û�
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
				cout << "�������" << endl;
				throw(ERROR_PASSWORD_WRONG);
			}
			
		}
	}
	if (find_user == false) {
		cout << "���û�������" << endl;
		throw(ERROR_USER_NOT_EXIST);

	}
}

//�ǳ��û�
void User_Logout()
{
	user_id = -1;
}

//�����û�
void User_Register(const char* user_name, const char* password)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "ֻ��root�û����Խ���ע��" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	int i;
	for (i = 0; i < USER_NUM; i++) {
		if (strcmp(user.u_name[i], user_name) == 0) {
			cout << "�û����Ѵ��ڣ��޷�ע��" << endl;
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
		cout << "�û����Ѵ������ƣ��޷�����ע��" << endl;
		throw(ERROR_USER_REGISTER);
	}
	Write_User(user);
}

//ɾ���û�
void User_Delete(const char* user_name)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "ֻ��root�û����Խ����û�ɾ��" << endl;
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
		cout << "���û�������" << endl;
		throw(ERROR_USER_NOT_EXIST);
	}

	Write_User(user);
}
//��ȡ��ǰ��¼���û����û������û�id
unsigned int Get_User(char* username)
{
	if (username != NULL) {
		User user;
		Read_User(user);
		strcpy(username, user.u_name[user_id]);
	}
	return user_id;
}
//�����û�����������
void Change_User_Group(const char* user_name,unsigned int user_group)
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "ֻ��root�û����Խ����û������" << endl;
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
		cout << "���û�������" << endl;
		throw(ERROR_USER_NOT_EXIST);
	}
	user.u_gid[i] = user_group;
	Write_User(user);
}
//��ʾ�û��б�
void Show_User_List()
{
	User user;
	Read_User(user);
	if (user_id != 0) {
		cout << "ֻ��root�û����Բ鿴�û��б�" << endl;
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