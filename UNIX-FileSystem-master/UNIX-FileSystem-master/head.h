#pragma once
#define _CRT_SECURE_NO_WARNINGS
//*******************各个h/cpp需要使用的库**************
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
# include <iomanip>
#include<bitset>
using namespace std;

//************************常量定义**********************
//--------------------- 文件 ---------------------------
//文件卷名称
static const string DISK_NAME = "myDisk.img";
//文件可以使用的最大节点数量
static const unsigned int NINODE = 10;
//子目录的最大数量
static const unsigned int SUBDIRECTORY_NUM = 12;
//文件名称的最大长度
static const unsigned int FILE_NAME_MAX = 32;
//--------------------- 用户 ---------------------------
//用户最大数量
static const unsigned int USER_NUM = 6;
//用户名称的最大长度
static const unsigned int USER_NAME_MAX = 16;
//用户密码的最大长度
static const unsigned int USER_PASSWORD_MAX = 32;
//--------------------- 空间分配 ---------------------------
//空闲盘块分组链式索引的一个链的大小
static const unsigned int FREE_BLOCK_GROUP_NUM = 50;
//Block大小
static const unsigned int BLOCK_SIZE = 512;
//Inode数量
static const unsigned int INODE_NUM = 256;
//SuperBlock大小
static const unsigned int SUPERBLOCK_SIZE = 512;
//SuperBlock开始的位置（以block为单位）
static const unsigned int SUPERBLOCK_POSITION = 1;
//Inode位示图大小
static const unsigned int INODE_BITMAP_SIZE = sizeof(int) * INODE_NUM;
//Inode位示图开始的位置（以block为单位）
static const unsigned int INODE_BITMAP_POSITION = int(SUPERBLOCK_POSITION + SUPERBLOCK_SIZE / BLOCK_SIZE);
//Inode大小
static const unsigned int INODE_SIZE = 64;
//Inode开始的位置（以block为单位）
static const unsigned int INODE_POSITION = int(INODE_BITMAP_POSITION + INODE_BITMAP_SIZE / BLOCK_SIZE);
//Block数量
static const unsigned int BLOCK_NUM =100000U;
//Block开始的位置（以block为单位）
static const unsigned int BLOCK_POSITION = int(INODE_POSITION + INODE_SIZE * INODE_NUM / BLOCK_SIZE);
//Directory位于的Inode块
static const unsigned int DIRECTORY_INODE = 2;




//************************数据结构**********************

//Inode结构体
struct Inode {

	enum INodeMode {
		IFILE = 0x1,//是文件
		IDIRECTORY = 0x2//是目录
	};
	enum INodePermission {//分为文件主、文件主同组和其他用户
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

	unsigned int i_addr[NINODE];//逻辑块号和物理块号转换的索引表
	unsigned int i_size;//文件大小，字节为单位
	unsigned short i_count;//引用计数
	unsigned short i_number;//Inode的编号
	unsigned short i_mode;//文件工作方式信息
	unsigned short i_permission;//文件权限
	unsigned short i_uid;//文件所有者的用户标识
	unsigned short i_gid;//文件所有者的组标识
	time_t i_time;//最后访问时间
};

//SuperBlock结构体
struct SuperBlock {
	unsigned short s_inodenum;//Inode总数
	unsigned short s_finodenum;//空闲Inode数
	unsigned short s_blocknum;//Block总数
	unsigned short s_fblocknum;//空闲Block数
	unsigned int s_nfree;//直接管理的空闲块数
	unsigned int s_free[FREE_BLOCK_GROUP_NUM];//空闲块索引表
};

//Directory结构体
struct Directory {
	unsigned int d_inodenumber[SUBDIRECTORY_NUM];//子目录Inode号
	char d_filename[SUBDIRECTORY_NUM][FILE_NAME_MAX];//子目录文件名
};

//User结构体
struct User {
	unsigned short u_id[USER_NUM];//用户id
	unsigned short u_gid[USER_NUM];//用户所在组id
	char u_name[USER_NUM][USER_NAME_MAX];     //用户名
	char u_password[USER_NUM][USER_PASSWORD_MAX]; //用户密码
};

//File结构体
struct File {
	unsigned int f_inodeid;//文件的inode编号
	unsigned int f_offset;//文件的读写指针位置
	unsigned int f_uid;//文件打开用户
};
//*******************************全局变量********************************
#ifdef MAIN
#define EXTERN    //定义变量
#else
#define EXTERN extern  //声明变量
#endif

EXTERN Directory directory;//当前目录
EXTERN fstream fd;//全局文件指针
EXTERN unsigned short user_id;//当前用户id

//*********************************函数操作*******************************
//--------------------tools-----------------------
//初始化整个文件系统空间
void Init();
//打开文件系统
void Activate();
//指令说明文档
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
//分配一个Block块
void Allocate_Block(unsigned int& block_num);
//释放一个Block块
void Free_Block(unsigned int block_num);
//读出SuperBlock块
void Read_SuperBlock(SuperBlock& superblock);
//写SuperBlock块
void Write_SuperBlock(SuperBlock& superblock);
//读出Bitmap块
void Read_InodeBitMap(unsigned int* inode_bitmap);
//写Bitmap块
void Write_InodeBitMap(unsigned int* inode_bitmap);
//读出User块
void Read_User(User& user);
//写User块
void Write_User(User& user);
//读出Inode块
void Read_Inode(Inode& inode, unsigned int pos);
//写Inode块
void Write_Inode(Inode& inode, unsigned int pos);
//Inode根据逻辑块号对应物理块号
void Get_Block_Pysical_Num(Inode& inode, unsigned int logical_num, unsigned int& physical_num_1, unsigned int& physical_num_2, unsigned int& physical_num_3);


//---------------------File---------------------
//创建一个文件
void Create_File(const char* file_name);
//删除一个文件
void Delete_File(const char* file_name);
//展示文件列表
void Show_File_List(bool detail);
//根据文件名称打开一个当前目录的文件
File* Open_File(const char* file_name);
//根据文件结构体关闭一个文件
void Close_File(File* file);
//写文件
unsigned int Write_File(File* file, const char* content);
//更改文件指针
void Seek_File(File* file, unsigned int pos);
//读文件
unsigned int Read_File(File* file, char* content);
//更改一个文件的权限
void Edit_File_Permission(const char* directory_name, unsigned short permission, bool add);

//------------------Directory----------------------
//创建一个目录
void Create_Directory(const char* directory_name);
//打开一个目录
void Open_Directory(const char* directory_name);
//获取当前目录
string Current_Directory();
//删除一个目录
void Remove_Directory(const char* directory_name);

//---------------------User------------------------
//登录用户
void User_Login(const char* user_name, const char* password);
//登出用户
void User_Logout();
//创建用户
void User_Register(const char* user_name, const char* password);
//获取当前登录的用户的用户名和用户id
unsigned int Get_User(char* username);
//删除用户
void User_Delete(const char* user_name);
// 更改用户的所属的组
void Change_User_Group(const char* user_name, unsigned int user_group);
//显示用户列表
void Show_User_List();