#pragma once
#define _CRT_SECURE_NO_WARNINGS
//*******************各个h/cpp需要使用的库**************
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
using namespace std;

//************************常量定义**********************
//--------------------- 文件 ---------------------------
//文件卷名称
static const string DISK_NAME = "myDisk.img";
//文件可以使用的最大节点数量
static const unsigned int NINODE = 16;
//子目录的最大数量
static const unsigned int SUBDIRECTORY_NUM = 16;
//文件名称的最大长度
static const unsigned int FILE_NAME_MAX = 32;
//--------------------- 用户 ---------------------------
//用户最大数量
static const unsigned int USER_NUM = 16;
//用户名称的最大长度
static const unsigned int USER_NAME_MAX = 32;
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
static const unsigned int INODE_SIZE = 128;
//Inode开始的位置（以block为单位）
static const unsigned int INODE_POSITION = int(INODE_BITMAP_POSITION + INODE_BITMAP_SIZE / BLOCK_SIZE);
//Block数量
static const unsigned int BLOCK_NUM = 500;
//Block开始的位置（以block为单位）
static const unsigned int BLOCK_POSITION = int(INODE_POSITION + INODE_SIZE * INODE_NUM / BLOCK_SIZE);



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
		OTHER_R = 04,
		OTHER_W = 02,
		OTHER_E = 01,
	};

	unsigned int i_number;//Inode的编号
	unsigned int i_addr[NINODE];//逻辑块号和物理块号转换的索引表
	unsigned short i_mode;//文件工作方式信息
	unsigned int i_count;//引用计数
	unsigned short i_permission;//文件权限
	unsigned short i_uid;//文件所有者的用户标识
	unsigned short i_gid;//文件所有者的组标识
	unsigned int i_size;//文件大小，字节为单位
	time_t time;//最后访问时间
};

//SuperBlock结构体
struct SuperBlock {
	unsigned short s_inodenum;//Inode总数
	unsigned short s_finodenum;//空闲Inode数
	unsigned short s_inodesize;//Inode大小
	unsigned short s_blocknum;//Block总数
	unsigned short s_fblocknum;//空闲Block数
	unsigned short s_blocksize;//Block大小
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
//********************************全局变量*****************************



//*********************************函数操作*******************************
//--------------------tools-----------------------
//初始化整个文件系统空间
bool Init();



//--------------------Block-----------------------
//分配一个Block块
void Allocate_Block(unsigned int& block_num);
//释放一个Block块
void Free_Block(unsigned int block_num);
//读出SuperBlock块
void Read_SuperBlock(SuperBlock& superblock);
//写SuperBlock块
void Write_SuperBlock(SuperBlock& superblock);