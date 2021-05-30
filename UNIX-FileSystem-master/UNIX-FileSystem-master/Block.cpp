//**********************有关Block的操作的函数***********************
#include "head.h"
#include "error.h"



//读出SuperBlock块
void Read_SuperBlock(SuperBlock& superblock)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&superblock, sizeof(superblock));
}

//写SuperBlock块
void Write_SuperBlock(SuperBlock& superblock)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
}

//读出Bitmap块
void Read_InodeBitMap(unsigned int *inode_bitmap)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)inode_bitmap, sizeof(unsigned int)* INODE_NUM);
}

//写Bitmap块
void Write_InodeBitMap(unsigned int* inode_bitmap)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);
}

//读出User块
void Read_User(User& user)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg((BLOCK_POSITION+1) * BLOCK_SIZE, ios::beg);
	fd.read((char*)&user,sizeof( user));
}

//写User块
void Write_User(User& user)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg((BLOCK_POSITION + 1) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
}

//读出Inode块
void Read_Inode(Inode& inode,unsigned int pos)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(INODE_POSITION* BLOCK_SIZE+pos*INODE_SIZE, ios::beg);
	fd.read((char*)&inode, sizeof(inode));
}

//写Inode块
void Write_Inode(Inode& inode, unsigned int pos)
{
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE + pos * INODE_SIZE, ios::beg);
	fd.write((char*)&inode, sizeof(inode));
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
		fd.seekg((BLOCK_POSITION + superblock.s_free[0])* BLOCK_SIZE  , ios::beg);
		fd.read((char*)&superblock.s_free, sizeof(superblock.s_free));
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