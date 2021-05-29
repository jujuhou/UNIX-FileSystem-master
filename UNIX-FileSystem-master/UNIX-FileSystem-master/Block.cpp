//**********************�й�Block�Ĳ����ĺ���***********************
#include "head.h"
#include "error.h"

//����SuperBlock��
void Read_SuperBlock(SuperBlock& superblock)
{
	fstream fd;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);

	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&superblock, sizeof(superblock));

	fd.close();
}

//дSuperBlock��
void Write_SuperBlock(SuperBlock& superblock)
{
	fstream fd;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);

	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));

	fd.close();
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
		fstream fd;
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + superblock.s_free[0])* BLOCK_SIZE  , ios::beg);
		fd.read((char*)&superblock.s_free, sizeof(superblock.s_free));
		fd.close();
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
		fstream fd;
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);

		fd.seekg((BLOCK_POSITION + block_num) * BLOCK_SIZE, ios::beg);
		fd.write((char*)&superblock.s_free, sizeof(superblock.s_free));
		fd.close();
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