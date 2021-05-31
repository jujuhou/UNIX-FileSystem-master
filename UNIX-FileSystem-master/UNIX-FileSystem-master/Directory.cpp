//**********************有关文件的操作的函数***********************
#include "head.h"
#include "error.h"


//创建一个目录
void Create_Directory(const char* directory_name)
{
	//检查目录名是否合法
	if (directory_name == NULL || strlen(directory_name) > FILENAME_MAX) {
		cout << "目录名不合法" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//取出superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	//是否还有空闲的block和inode
	if (superblock.s_fblocknum <= 0 || superblock.s_finodenum <= 0)
	{
		cout << "空间不足，无法创建目录" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//检测当前目录是否有同名目录
	for (int i = 0; i < SUBDIRECTORY_NUM; i++)
	{
		if (strcmp(directory.d_filename[i], directory_name) == 0)
		{
			Inode tmp_inode;
			int tmp_inode_number = directory.d_inodenumber[i];

			fd.seekg(INODE_POSITION * BLOCK_SIZE + tmp_inode_number * INODE_SIZE, ios::beg);
			fd.read((char*)&tmp_inode, sizeof(tmp_inode));
			if (tmp_inode.i_mode & Inode::IFILE)
				continue;
			else {
				cout << "文件名不合法，当前目录已存在名为" << directory_name << "的文件" << endl;
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
		cout << "当前目录已达" << SUBDIRECTORY_NUM << "的数量限制，无法继续创建目录" << endl;
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

	//寻找新的block
	unsigned int block_num;
	Allocate_Block(block_num);

	//读出User结构体
	User user;
	Read_User(user);
	//创建新的Inode
	Inode new_inode;
	new_inode.i_number = new_inode_index;//Inode的编号
	new_inode.i_mode = Inode::IDIRECTORY;//文件
	new_inode.i_size = 0;
	new_inode.i_uid = user_id;
	new_inode.i_gid = user.u_gid[user_id];
	new_inode.i_addr[0] = block_num;
	new_inode.i_permission = 0777;
	new_inode.i_count = 0;
	new_inode.i_time = time(NULL);
	//写入Inode到内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));

	//更新位示图
	inode_bitmap[new_inode.i_number] = 1;
	Write_InodeBitMap(inode_bitmap);

	//将directory写入到block
	Directory new_directory;
	strcpy(new_directory.d_filename[0], ".");
	strcpy(new_directory.d_filename[1], "..");
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		new_directory.d_filename[i][0] = '\0';
	}
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		new_directory.d_inodenumber[i] = -1;
	}
	new_directory.d_inodenumber[0] = new_inode_index;
	new_directory.d_inodenumber[1] = directory.d_inodenumber[0];
	fd.seekg((block_num+BLOCK_POSITION)*BLOCK_SIZE,ios::beg);
	fd.write((char*)&new_directory, sizeof(new_directory));

	//更新目录
	int directory_inode_index = directory.d_inodenumber[0];//"."
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_index);
	//加入当前目录
	for (int i = 2; i < SUBDIRECTORY_NUM; i++)
	{
		if (strlen(directory.d_filename[i]) == 0)
		{
			strcat(directory.d_filename[i], directory_name);
			directory.d_inodenumber[i] = new_inode.i_number;
			break;
		}
	}
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));

	//更新超级块
	Read_SuperBlock(superblock);
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);

}

//打开一个目录
void Open_Directory(const char* directory_name)
{
	if (directory_name == NULL) {
		cout << "路径名不合法" << endl;
		throw(ERROR_INVALID_PATH);
	}
	char *reached_path=new char[strlen(directory_name) +2];
	reached_path[0] = '\0';

	int index=0;
	while (strlen(reached_path) < strlen(directory_name)){
		char to_dir[FILE_NAME_MAX];
		for (int i=0;;i++ ,index++) {
			if (directory_name[index] == '\\' || directory_name[index] == '/'|| directory_name[index] == '\0') {
				to_dir[i] = '\0';
				index++;
				break;
			}
			to_dir[i] = directory_name[index];
		}
		bool find=false;
		//遍历目录
		for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
			if (strcmp(to_dir, directory.d_filename[i]) == 0) {
				Inode to_inode;
				Read_Inode(to_inode, directory.d_inodenumber[i]);

				if (to_inode.i_mode == Inode::IDIRECTORY) {
					find = true;
					strcat(reached_path, to_dir);
					strcat(reached_path, "/");

					//更改directory
					fd.seekg(BLOCK_SIZE * (BLOCK_POSITION + to_inode.i_addr[0]), ios::beg);
					fd.read((char*)&directory, sizeof(directory));
					break;
				}
			}
		}
		if (find == false) {
			cout << "目前已进入路径：" << reached_path << "，目录 " << to_dir << "不存在" << endl;
			throw(ERROR_INVALID_PATH);
		}
	}

	delete [] reached_path;
}