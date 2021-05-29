//**********************有关Block的操作的函数***********************
#include "head.h"
#include "error.h"

//读出SuperBlock块
void Read_SuperBlock(SuperBlock& superblock)
{
	fstream fd;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);

	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&superblock, sizeof(superblock));

	fd.close();
}

//写SuperBlock块
void Write_SuperBlock(SuperBlock& superblock)
{
	fstream fd;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);

	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));

	fd.close();
}


//分配一个Block块
void Allocate_Block(unsigned int& block_num)
{
	SuperBlock superblock;
	//从内存读出
	Read_SuperBlock(superblock);

	//如果没有free的了
	if (superblock.s_nfree == 0) {
		if (superblock.s_free[0] == 0) {
			cout << "没有空间继续分配一个Block！";
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
	//写入内存
	Write_SuperBlock(superblock);
}
//释放一个Block块
void Free_Block(unsigned int block_num)
{
	SuperBlock superblock;
	//从内存读出
	Read_SuperBlock(superblock);

	//如果该空闲块已满，需要使superblock指向一个新的空块
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