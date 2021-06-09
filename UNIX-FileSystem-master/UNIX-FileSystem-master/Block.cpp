//**********************�й�Block�Ĳ����ĺ���***********************
#include "head.h"
#include "error.h"



//����SuperBlock��
void Read_SuperBlock(SuperBlock& superblock)
{
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

//����Bitmap��
void Read_InodeBitMap(unsigned int *inode_bitmap)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)inode_bitmap, sizeof(unsigned int)* INODE_NUM);
	fd.close();
}

//дBitmap��
void Write_InodeBitMap(unsigned int* inode_bitmap)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);
	fd.close();
}

//����User��
void Read_User(User& user)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg((BLOCK_POSITION+1) * BLOCK_SIZE, ios::beg);
	fd.read((char*)&user,sizeof( user));
	fd.close();
}

//дUser��
void Write_User(User& user)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg((BLOCK_POSITION + 1) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
	fd.close();
}

//����Inode��
void Read_Inode(Inode& inode,unsigned int pos)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//���ڴ����
	fd.seekg(INODE_POSITION* BLOCK_SIZE+pos*INODE_SIZE, ios::beg);
	fd.read((char*)&inode, sizeof(inode));
	fd.close();
}

//дInode��
void Write_Inode(Inode& inode, unsigned int pos)
{
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//д���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE + pos * INODE_SIZE, ios::beg);
	fd.write((char*)&inode, sizeof(inode));
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

//Inode�����߼���Ŷ�Ӧ�����ţ�pysical_num_1Ϊʵ�������ţ�pysical_num_2Ϊ����һ�����������ţ�pysical_num_3Ϊ�����ϼ����������ţ���
void Get_Block_Pysical_Num(Inode& inode, unsigned int logical_num,unsigned int& physical_num_1, unsigned int& physical_num_2, unsigned int& physical_num_3 )
{
	if (logical_num <= 5) {
		physical_num_1 = inode.i_addr[logical_num];
	}
	else if (logical_num <= (BLOCK_SIZE / sizeof(unsigned int))*2+5) {
		unsigned int block_map_1_index=(logical_num-6)/(BLOCK_SIZE / sizeof(unsigned int))+6;
		unsigned int block_map_1[BLOCK_SIZE / sizeof(unsigned int)];
		//���ڴ����
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION+ inode.i_addr[block_map_1_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_1, sizeof(block_map_1));
		fd.close();

		//�ҵ���Ӧ�Ŀ��
		physical_num_1 = block_map_1[(logical_num - 6) % (BLOCK_SIZE / sizeof(unsigned int))];
		physical_num_2 = inode.i_addr[block_map_1_index];
	}
	else if (logical_num <= (BLOCK_SIZE / sizeof(unsigned int)) * (BLOCK_SIZE / sizeof(unsigned int)) * 2+(BLOCK_SIZE / sizeof(unsigned int)) * 2 + 5) {
		unsigned int block_map_2_index = (logical_num - 262) / (BLOCK_SIZE / sizeof(unsigned int)) / (BLOCK_SIZE / sizeof(unsigned int))+8;
		unsigned int block_map_2[BLOCK_SIZE / sizeof(unsigned int)];

		//���ڴ����
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + inode.i_addr[block_map_2_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_2, sizeof(block_map_2));
		fd.close();

		unsigned int block_map_1_index = ((logical_num - 262) / (BLOCK_SIZE / sizeof(unsigned int)))% (BLOCK_SIZE / sizeof(unsigned int));
		unsigned int block_map_1[BLOCK_SIZE / sizeof(unsigned int)];
		
		//���ڴ����
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + block_map_2[block_map_1_index]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&block_map_1, sizeof(block_map_1));
		fd.close();

		//�ҵ���Ӧ�Ŀ��
		physical_num_1 = block_map_1[(logical_num - 262) % (BLOCK_SIZE / sizeof(unsigned int))];
		physical_num_2 = block_map_2[block_map_1_index];
		physical_num_3 = inode.i_addr[block_map_2_index];
	}
	else {
		cout << "�߼���ų�������" << endl;
		throw(ERROT_OUT_OF_RANGE);
	}
}