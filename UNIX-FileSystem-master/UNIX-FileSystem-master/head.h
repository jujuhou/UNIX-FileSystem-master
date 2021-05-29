#pragma once
#define _CRT_SECURE_NO_WARNINGS
//*******************����h/cpp��Ҫʹ�õĿ�**************
# include <iostream>
# include <fstream>
# include <string>
# include <time.h>
using namespace std;

//************************��������**********************
//--------------------- �ļ� ---------------------------
//�ļ�������
static const string DISK_NAME = "myDisk.img";
//�ļ�����ʹ�õ����ڵ�����
static const unsigned int NINODE = 16;
//��Ŀ¼���������
static const unsigned int SUBDIRECTORY_NUM = 16;
//�ļ����Ƶ���󳤶�
static const unsigned int FILE_NAME_MAX = 32;
//--------------------- �û� ---------------------------
//�û��������
static const unsigned int USER_NUM = 16;
//�û����Ƶ���󳤶�
static const unsigned int USER_NAME_MAX = 32;
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
static const unsigned int INODE_SIZE = 128;
//Inode��ʼ��λ�ã���blockΪ��λ��
static const unsigned int INODE_POSITION = int(INODE_BITMAP_POSITION + INODE_BITMAP_SIZE / BLOCK_SIZE);
//Block����
static const unsigned int BLOCK_NUM = 500;
//Block��ʼ��λ�ã���blockΪ��λ��
static const unsigned int BLOCK_POSITION = int(INODE_POSITION + INODE_SIZE * INODE_NUM / BLOCK_SIZE);



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
		OTHER_R = 04,
		OTHER_W = 02,
		OTHER_E = 01,
	};

	unsigned int i_number;//Inode�ı��
	unsigned int i_addr[NINODE];//�߼���ź�������ת����������
	unsigned short i_mode;//�ļ�������ʽ��Ϣ
	unsigned int i_count;//���ü���
	unsigned short i_permission;//�ļ�Ȩ��
	unsigned short i_uid;//�ļ������ߵ��û���ʶ
	unsigned short i_gid;//�ļ������ߵ����ʶ
	unsigned int i_size;//�ļ���С���ֽ�Ϊ��λ
	time_t time;//������ʱ��
};

//SuperBlock�ṹ��
struct SuperBlock {
	unsigned short s_inodenum;//Inode����
	unsigned short s_finodenum;//����Inode��
	unsigned short s_inodesize;//Inode��С
	unsigned short s_blocknum;//Block����
	unsigned short s_fblocknum;//����Block��
	unsigned short s_blocksize;//Block��С
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
//********************************ȫ�ֱ���*****************************



//*********************************��������*******************************
//--------------------tools-----------------------
//��ʼ�������ļ�ϵͳ�ռ�
bool Init();



//--------------------Block-----------------------
//����һ��Block��
void Allocate_Block(unsigned int& block_num);
//�ͷ�һ��Block��
void Free_Block(unsigned int block_num);
//����SuperBlock��
void Read_SuperBlock(SuperBlock& superblock);
//дSuperBlock��
void Write_SuperBlock(SuperBlock& superblock);