#pragma once
#define _CRT_SECURE_NO_WARNINGS
//*******************����h/cpp��Ҫʹ�õĿ�**************
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
# include <iomanip>
#include<bitset>
using namespace std;

//************************��������**********************
//--------------------- �ļ� ---------------------------
//�ļ�������
static const string DISK_NAME = "myDisk.img";
//�ļ�����ʹ�õ����ڵ�����
static const unsigned int NINODE = 10;
//��Ŀ¼���������
static const unsigned int SUBDIRECTORY_NUM = 12;
//�ļ����Ƶ���󳤶�
static const unsigned int FILE_NAME_MAX = 32;
//--------------------- �û� ---------------------------
//�û��������
static const unsigned int USER_NUM = 6;
//�û����Ƶ���󳤶�
static const unsigned int USER_NAME_MAX = 16;
//�û��������󳤶�
static const unsigned int USER_PASSWORD_MAX = 32;
//--------------------- �ռ���� ---------------------------
//�����̿������ʽ������һ�����Ĵ�С
static const unsigned int FREE_BLOCK_GROUP_NUM = 50;
//Block��С
static const unsigned int BLOCK_SIZE = 512;
//Inode����
static const unsigned int INODE_NUM = 256;
//SuperBlock��С
static const unsigned int SUPERBLOCK_SIZE = 512;
//SuperBlock��ʼ��λ�ã���blockΪ��λ��
static const unsigned int SUPERBLOCK_POSITION = 1;
//Inodeλʾͼ��С
static const unsigned int INODE_BITMAP_SIZE = sizeof(int) * INODE_NUM;
//Inodeλʾͼ��ʼ��λ�ã���blockΪ��λ��
static const unsigned int INODE_BITMAP_POSITION = int(SUPERBLOCK_POSITION + SUPERBLOCK_SIZE / BLOCK_SIZE);
//Inode��С
static const unsigned int INODE_SIZE = 64;
//Inode��ʼ��λ�ã���blockΪ��λ��
static const unsigned int INODE_POSITION = int(INODE_BITMAP_POSITION + INODE_BITMAP_SIZE / BLOCK_SIZE);
//Block����
static const unsigned int BLOCK_NUM =100000U;
//Block��ʼ��λ�ã���blockΪ��λ��
static const unsigned int BLOCK_POSITION = int(INODE_POSITION + INODE_SIZE * INODE_NUM / BLOCK_SIZE);
//Directoryλ�ڵ�Inode��
static const unsigned int DIRECTORY_INODE = 2;




//************************���ݽṹ**********************

//Inode�ṹ��
struct Inode {

	enum INodeMode {
		IFILE = 0x1,//���ļ�
		IDIRECTORY = 0x2//��Ŀ¼
	};
	enum INodePermission {//��Ϊ�ļ������ļ���ͬ��������û�
		OWNER_R = 0400,
		OWNER_W = 0200,
		OWNER_E = 0100,
		GROUP_R = 040,
		GROUP_W = 020,
		GROUP_E = 010,
		ELSE_R = 04,
		ELSE_W = 02,
		ELSE_E = 01,
	};

	unsigned int i_addr[NINODE];//�߼���ź�������ת����������
	unsigned int i_size;//�ļ���С���ֽ�Ϊ��λ
	unsigned short i_count;//���ü���
	unsigned short i_number;//Inode�ı��
	unsigned short i_mode;//�ļ�������ʽ��Ϣ
	unsigned short i_permission;//�ļ�Ȩ��
	unsigned short i_uid;//�ļ������ߵ��û���ʶ
	unsigned short i_gid;//�ļ������ߵ����ʶ
	time_t i_time;//������ʱ��
};

//SuperBlock�ṹ��
struct SuperBlock {
	unsigned short s_inodenum;//Inode����
	unsigned short s_finodenum;//����Inode��
	unsigned short s_blocknum;//Block����
	unsigned short s_fblocknum;//����Block��
	unsigned int s_nfree;//ֱ�ӹ���Ŀ��п���
	unsigned int s_free[FREE_BLOCK_GROUP_NUM];//���п�������
};

//Directory�ṹ��
struct Directory {
	unsigned int d_inodenumber[SUBDIRECTORY_NUM];//��Ŀ¼Inode��
	char d_filename[SUBDIRECTORY_NUM][FILE_NAME_MAX];//��Ŀ¼�ļ���
};

//User�ṹ��
struct User {
	unsigned short u_id[USER_NUM];//�û�id
	unsigned short u_gid[USER_NUM];//�û�������id
	char u_name[USER_NUM][USER_NAME_MAX];     //�û���
	char u_password[USER_NUM][USER_PASSWORD_MAX]; //�û�����
};

//File�ṹ��
struct File {
	unsigned int f_inodeid;//�ļ���inode���
	unsigned int f_offset;//�ļ��Ķ�дָ��λ��
	unsigned int f_uid;//�ļ����û�
};
//*******************************ȫ�ֱ���********************************
#ifdef MAIN
#define EXTERN    //�������
#else
#define EXTERN extern  //��������
#endif

EXTERN Directory directory;//��ǰĿ¼
EXTERN fstream fd;//ȫ���ļ�ָ��
EXTERN unsigned short user_id;//��ǰ�û�id

//*********************************��������*******************************
//--------------------tools-----------------------
//��ʼ�������ļ�ϵͳ�ռ�
void Init();
//���ļ�ϵͳ
void Activate();
//ָ��˵���ĵ�
void help();
void help_attrib();
void help_cd();
void help_del();
void help_dir();
void help_exit();
void help_mkdir();
void help_rmdir();
void help_print();
void help_write();
void help_open();
void help_close();
void help_fseek();
void help_create();
void help_logout();
void help_whoami();
void help_format();
void help_register();
void help_deleteaccount();
void help_su();
void help_chgrp();
void help_userlist();
void help_openlist();



//--------------------Block-----------------------
//����һ��Block��
void Allocate_Block(unsigned int& block_num);
//�ͷ�һ��Block��
void Free_Block(unsigned int block_num);
//����SuperBlock��
void Read_SuperBlock(SuperBlock& superblock);
//дSuperBlock��
void Write_SuperBlock(SuperBlock& superblock);
//����Bitmap��
void Read_InodeBitMap(unsigned int* inode_bitmap);
//дBitmap��
void Write_InodeBitMap(unsigned int* inode_bitmap);
//����User��
void Read_User(User& user);
//дUser��
void Write_User(User& user);
//����Inode��
void Read_Inode(Inode& inode, unsigned int pos);
//дInode��
void Write_Inode(Inode& inode, unsigned int pos);
//Inode�����߼���Ŷ�Ӧ������
void Get_Block_Pysical_Num(Inode& inode, unsigned int logical_num, unsigned int& physical_num_1, unsigned int& physical_num_2, unsigned int& physical_num_3);


//---------------------File---------------------
//����һ���ļ�
void Create_File(const char* file_name);
//ɾ��һ���ļ�
void Delete_File(const char* file_name);
//չʾ�ļ��б�
void Show_File_List(bool detail);
//�����ļ����ƴ�һ����ǰĿ¼���ļ�
File* Open_File(const char* file_name);
//�����ļ��ṹ��ر�һ���ļ�
void Close_File(File* file);
//д�ļ�
unsigned int Write_File(File* file, const char* content);
//�����ļ�ָ��
void Seek_File(File* file, unsigned int pos);
//���ļ�
unsigned int Read_File(File* file, char* content);
//����һ���ļ���Ȩ��
void Edit_File_Permission(const char* directory_name, unsigned short permission, bool add);

//------------------Directory----------------------
//����һ��Ŀ¼
void Create_Directory(const char* directory_name);
//��һ��Ŀ¼
void Open_Directory(const char* directory_name);
//��ȡ��ǰĿ¼
string Current_Directory();
//ɾ��һ��Ŀ¼
void Remove_Directory(const char* directory_name);

//---------------------User------------------------
//��¼�û�
void User_Login(const char* user_name, const char* password);
//�ǳ��û�
void User_Logout();
//�����û�
void User_Register(const char* user_name, const char* password);
//��ȡ��ǰ��¼���û����û������û�id
unsigned int Get_User(char* username);
//ɾ���û�
void User_Delete(const char* user_name);
// �����û�����������
void Change_User_Group(const char* user_name, unsigned int user_group);
//��ʾ�û��б�
void Show_User_List();