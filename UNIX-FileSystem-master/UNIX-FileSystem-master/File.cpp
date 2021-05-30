//**********************有关文件的操作的函数***********************
#include "head.h"
#include "error.h"


//创建一个文件
void Create_File(const char* file_name)
{
	//检查文件名是否合法
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "文件名不合法" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//取出superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	//是否还有空闲的block和inode
	if (superblock.s_fblocknum <= 0 || superblock.s_finodenum <= 0)
	{
		cout << "空间不足，无法创建新文件" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//检测当前目录是否有同名文件
	for (int i = 0; i < SUBDIRECTORY_NUM; i++)
	{
		if (strcmp(directory.d_filename[i], file_name) == 0)
		{
			Inode tmp_inode;
			int tmp_inode_number = directory.d_inodenumber[i];

			fd.seekg(INODE_POSITION * BLOCK_SIZE + tmp_inode_number * INODE_SIZE, ios::beg);
			fd.read((char*)&tmp_inode,sizeof(tmp_inode));
			if (tmp_inode.i_mode&Inode::IDIRECTORY) 
				continue;
			else {
				cout << "文件名不合法，当前目录已存在名为" << file_name << "的文件" << endl;
				throw(ERROR_INVALID_FILENAME);
			}
		}
	}

	//检测当前目录的文件数量是否达到限制
	int itemCounter = 0;
	for (int i = 0; i < SUBDIRECTORY_NUM; i++)
		if (strlen(directory.d_filename[i]) > 0)
			itemCounter++;

	if (itemCounter == SUBDIRECTORY_NUM)
	{
		cout << "当前目录已达" << SUBDIRECTORY_NUM << "的数量限制，无法继续创建文件" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//寻找新的inode
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);

	int new_inode_index = 0;
	unsigned int new_block_addr = -1;
	for (; new_inode_index < INODE_NUM; new_inode_index++)
		if (inode_bitmap[new_inode_index] == 0)
			break;

	//读出User结构体
	User user;
	Read_User(user);
	//创建新的Inode
	Inode new_inode;
	new_inode.i_number = new_inode_index;//Inode的编号
	new_inode.i_mode = Inode::IFILE;//文件
	new_inode.i_size = 0;
	new_inode.i_uid = user_id;
	new_inode.i_gid = user.u_gid[user_id];
	new_inode.i_permission = 0777;
	new_inode.i_count = 0;
	new_inode.time = time(NULL);
	//写入Inode到内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));

	//更新位示图
	inode_bitmap[new_inode.i_number] = 1;
	Write_InodeBitMap(inode_bitmap);

	//更新目录
	int directory_inode_index = directory.d_inodenumber[0];//"."
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_index);
	//加入当前目录
	for (int i = 2; i < SUBDIRECTORY_NUM; i++)
	{
		if (strlen(directory.d_filename[i]) == 0)
		{
			strcat(directory.d_filename[i], file_name);
			directory.d_inodenumber[i] = new_inode.i_number;
			break;
		}
	}
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0])*BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));

	//更新超级块
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);
}

//展示文件列表
void Show_File_List()
{
	const int file_name_length = 25, 
		type_length=15,
		user_id_length = 15, 
		user_group_length = 15, 
		inode_id_length = 15, 
		size_length = 15, 
		permission_length = 15, 
		time_length = 25;
	cout << setw(file_name_length) << "File/Directory Name" <<
		setw(type_length) << "Type" <<
		setw(user_id_length) << "Owner Id" <<
		setw(user_group_length) << "Owner Group" <<
		setw(inode_id_length) << "Inode Id" <<
		setw(size_length) << "File Size(Block)" <<
		setw(permission_length) << "Permission" <<
		setw(time_length) << "Edit Time" << endl;

	for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
		if (strlen(directory.d_filename[i]) > 0) {
			Inode inode;
			Read_Inode(inode, directory.d_inodenumber[i]);

			char time[80];
			strftime(time, 80, "%Y-%m-%d %H:%M:%S", localtime(&inode.time));

			cout << setw(file_name_length) << directory.d_filename[i]<<
				setw(type_length) << ((inode.i_mode & Inode::IDIRECTORY) ? "DIRECTORY" : "FILE")<<
				setw(user_id_length) << inode.i_uid <<
				setw(user_group_length) << inode.i_gid <<
				setw(inode_id_length) << inode.i_number <<
				setw(size_length) << inode.i_size <<
				setw(permission_length) << bitset<9>(inode.i_permission) <<
				setw(time_length) << time<< endl;
		}
	}

}