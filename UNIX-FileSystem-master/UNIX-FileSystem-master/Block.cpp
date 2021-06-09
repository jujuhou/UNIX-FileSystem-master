//**********************有关Block的操作的函数***********************
#include "head.h"
#include "error.h"



//读出SuperBlock块
void Read_SuperBlock(SuperBlock& superblock)
{
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

//读出Bitmap块
void Read_InodeBitMap(unsigned int *inode_bitmap)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)inode_bitmap, sizeof(unsigned int)* INODE_NUM);
	fd.close();
}

//写Bitmap块
void Write_InodeBitMap(unsigned int* inode_bitmap)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);
	fd.close();
}

//读出User块
void Read_User(User& user)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg((BLOCK_POSITION+1) * BLOCK_SIZE, ios::beg);
	fd.read((char*)&user,sizeof( user));
	fd.close();
}

//写User块
void Write_User(User& user)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg((BLOCK_POSITION + 1) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
	fd.close();
}

//读出Inode块
void Read_Inode(Inode& inode,unsigned int pos)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//从内存读出
	fd.seekg(INODE_POSITION* BLOCK_SIZE+pos*INODE_SIZE, ios::beg);
	fd.read((char*)&inode, sizeof(inode));
	fd.close();
}

//写Inode块
void Write_Inode(Inode& inode, unsigned int pos)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE + pos * INODE_SIZE, ios::beg);
	fd.write((char*)&inode, sizeof(inode));
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
		block_num = superblock.s_free[0];
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + superblock.s_free[0])* BLOCK_SIZE  , ios::beg);
		fd.read((char*)&superblock.s_free, sizeof(superblock.s_free));
		fd.close();
		superblock.s_nfree = FREE_BLOCK_GROUP_NUM-1;
	}
	else {
		block_num = superblock.s_free[superblock.s_nfree];
		superblock.s_nfree--;
		superblock.s_fblocknum--;
	}
	
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

//Inode根据逻辑块号对应物理块号（pysical_num_1为实际物理块号，pysical_num_2为其上一级索引物理块号，pysical_num_3为其上上级索引物理块号，）
void Get_Block_Pysical_Num(Inode& inode, unsigned int logical_num,unsigned int& physical_num_1, unsigned int& physical_num_2, unsigned int& physical_num_3 )
{
	if (logical_num <= 5) {
		physical_num_1 = inode.i_addr[logical_num];
	}
	else if (logical_num <= (BLOCK_SIZE / sizeof(unsigned int))*2+5) {
		unsigned int block_map_1_index=(logical_num-6)/(BLOCK_SIZE / sizeof(unsigned int))+6;
		unsigned int block_map_1[BLOCK_SIZE / sizeof(unsigned int)];
		//从内存读出
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION+ inode.i_addr[block_map_1_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_1, sizeof(block_map_1));
		fd.close();

		//找到相应的块号
		physical_num_1 = block_map_1[(logical_num - 6) % (BLOCK_SIZE / sizeof(unsigned int))];
		physical_num_2 = inode.i_addr[block_map_1_index];
	}
	else if (logical_num <= (BLOCK_SIZE / sizeof(unsigned int)) * (BLOCK_SIZE / sizeof(unsigned int)) * 2+(BLOCK_SIZE / sizeof(unsigned int)) * 2 + 5) {
		unsigned int block_map_2_index = (logical_num - 262) / (BLOCK_SIZE / sizeof(unsigned int)) / (BLOCK_SIZE / sizeof(unsigned int))+8;
		unsigned int block_map_2[BLOCK_SIZE / sizeof(unsigned int)];

		//从内存读出
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + inode.i_addr[block_map_2_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_2, sizeof(block_map_2));
		fd.close();

		unsigned int block_map_1_index = ((logical_num - 262) / (BLOCK_SIZE / sizeof(unsigned int)))% (BLOCK_SIZE / sizeof(unsigned int));
		unsigned int block_map_1[BLOCK_SIZE / sizeof(unsigned int)];
		
		//从内存读出
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + block_map_2[block_map_1_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_1, sizeof(block_map_1));
		fd.close();

		//找到相应的块号
		physical_num_1 = block_map_1[(logical_num - 262) % (BLOCK_SIZE / sizeof(unsigned int))];
		physical_num_2 = block_map_2[block_map_1_index];
		physical_num_3 = inode.i_addr[block_map_2_index];
	}
	else {
		cout << "逻辑块号超出索引" << endl;
		throw(ERROT_OUT_OF_RANGE);
	}
}