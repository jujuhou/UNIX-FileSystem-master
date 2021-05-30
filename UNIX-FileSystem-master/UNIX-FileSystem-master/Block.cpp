//**********************�й�Block�Ĳ����ĺ���***********************
#include "head.h"
#include "error.h"



//����SuperBlock��
void Read_SuperBlock(SuperBlock& superblock)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&superblock, sizeof(superblock));
}

//дSuperBlock��
void Write_SuperBlock(SuperBlock& superblock)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
}

//����Bitmap��
void Read_InodeBitMap(unsigned int *inode_bitmap)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)inode_bitmap, sizeof(unsigned int)* INODE_NUM);
}

//дBitmap��
void Write_InodeBitMap(unsigned int* inode_bitmap)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);
}

//����User��
void Read_User(User& user)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg((BLOCK_POSITION+1) * BLOCK_SIZE, ios::beg);
	fd.read((char*)&user,sizeof( user));
}

//дUser��
void Write_User(User& user)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg((BLOCK_POSITION + 1) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
}

//����Inode��
void Read_Inode(Inode& inode,unsigned int pos)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(INODE_POSITION* BLOCK_SIZE+pos*INODE_SIZE, ios::beg);
	fd.read((char*)&inode, sizeof(inode));
}

//дInode��
void Write_Inode(Inode& inode, unsigned int pos)
{
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE + pos * INODE_SIZE, ios::beg);
	fd.write((char*)&inode, sizeof(inode));
}

//����һ��Block��
void Allocate_Block(unsigned int& block_num)
{
	SuperBlock superblock;
	//���ڴ����
	Read_SuperBlock(superblock);

	//���û��free����
	if (superblock.s_nfree == 0) {
		if (superblock.s_free[0] == 0) {
			cout << "û�пռ��������һ��Block��";
			throw(ERROR_OUT_OF_SPACE);
		}
		fd.seekg((BLOCK_POSITION + superblock.s_free[0])* BLOCK_SIZE  , ios::beg);
		fd.read((char*)&superblock.s_free, sizeof(superblock.s_free));
		superblock.s_nfree = FREE_BLOCK_GROUP_NUM - 1;
	}
	block_num = superblock.s_free[superblock.s_nfree];
	superblock.s_nfree--;
	superblock.s_fblocknum--;
	//д���ڴ�
	Write_SuperBlock(superblock);
}
//�ͷ�һ��Block��
void Free_Block(unsigned int block_num)
{
	SuperBlock superblock;
	//���ڴ����
	Read_SuperBlock(superblock);

	//����ÿ��п���������Ҫʹsuperblockָ��һ���µĿտ�
	if (superblock.s_nfree == FREE_BLOCK_GROUP_NUM - 1) {
		fd.seekg((BLOCK_POSITION + block_num) * BLOCK_SIZE, ios::beg);
		fd.write((char*)&superblock.s_free, sizeof(superblock.s_free));
		superblock.s_free[0] = block_num;
		superblock.s_nfree = 0;
		superblock.s_fblocknum++;
	}
	else {
		superblock.s_nfree++;
		superblock.s_free[superblock.s_nfree] = block_num;
	}
	Write_SuperBlock(superblock);
}