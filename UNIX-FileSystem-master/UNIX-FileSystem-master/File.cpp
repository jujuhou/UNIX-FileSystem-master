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

			fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
			fd.seekg(INODE_POSITION * BLOCK_SIZE + tmp_inode_number * INODE_SIZE, ios::beg);
			fd.read((char*)&tmp_inode,sizeof(tmp_inode));
			fd.close();
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
	new_inode.i_time = time(NULL);
	//写入Inode到内存
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));
	fd.close();

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
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0])*BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));
	fd.close();

	//更新超级块
	Read_SuperBlock(superblock);
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);
}

//删除一个文件
void Delete_File(const char* file_name)
{
	//检查文件名是否合法
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "文件名不合法" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//取出superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	Inode inode;
	int inode_num;
	int file_index_in_directory;
	bool exist = false;
	//检测当前目录是否有同名文件
	for (int i = 0; i < SUBDIRECTORY_NUM; i++){
		if (strcmp(directory.d_filename[i], file_name) == 0){
			inode_num = directory.d_inodenumber[i];
			Read_Inode(inode, inode_num);
			if (inode.i_mode & Inode::IFILE) {
				exist = true;
				file_index_in_directory = i;
			}
		}
	}
	if (exist == false) {
		cout << "文件名不合法，当前目录不存在名为" << file_name << "的文件" << endl;
		throw(ERROR_INVALID_FILENAME);
	}

	//检查删除的权限（删除需要上一级目录的写权限）
	//首先查找上一级目录
	//读出User结构体
	User user;
	Read_User(user);
	int directory_inode_num = directory.d_inodenumber[0];
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_num);
	if (user_id == inode.i_uid && !(directory_inode.i_permission & Inode::OWNER_W)) {
		cout << "您没有权限删除该文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid&&user.u_gid[user_id] == inode.i_gid && !(directory_inode.i_permission & Inode::GROUP_W)) {
		cout << "您没有权限删除该文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid)&& !(directory_inode.i_permission & Inode::ELSE_W)){
		cout << "您没有权限删除该文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	//检查文件是否已被用户打开
	if (inode.i_count > 0) {
		cout << "当前文件已被打开，无法删除" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	
	//开始进行删除
	//首先释放所有的block
	while (inode.i_size > 0) {
		unsigned int block_num_logical = (inode.i_size-1) / BLOCK_SIZE;
		unsigned int block_num_physical_1,block_num_physical_2, block_num_physical_3;
		Get_Block_Pysical_Num(inode,block_num_logical, block_num_physical_1, block_num_physical_2, block_num_physical_3);
		
		inode.i_size-= BLOCK_SIZE;
		Free_Block(block_num_physical_1);
		if (block_num_logical >= 6 && block_num_logical <= 261 && (block_num_logical - 6) % 128 == 0) {
			Free_Block(block_num_physical_2);
		}
		if (block_num_logical >= 262 && (block_num_logical - 262) % 128 == 0) {
			Free_Block(block_num_physical_2);
		}
		if (block_num_logical >= 262 && (block_num_logical - 262) % (128*128) == 0) {
			Free_Block(block_num_physical_3);
		}
	}
	//更新位示图（释放Inode）
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);
	inode_bitmap[inode.i_number] = 0;
	Write_InodeBitMap(inode_bitmap);

	//更新目录
	directory.d_filename[file_index_in_directory][0] = '\0';
	directory.d_inodenumber[file_index_in_directory] = -1;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));
	fd.close();

	//更新超级块
	superblock.s_finodenum++;
	Write_SuperBlock(superblock);
}


//展示文件列表
void Show_File_List(bool detail)
{
	const int file_name_length = 25, 
		type_length=15,
		user_id_length = 15, 
		user_group_length = 15, 
		inode_id_length = 15, 
		size_length = 15, 
		permission_length = 20, 
		time_length = 25;
	
	cout << setw(time_length) << "Edit Time" <<
		setw(type_length) << "Type" <<
		setw(size_length) << "Size(Byte)"<<
		setw(file_name_length) << "File/Directory Name";
	if (detail) {
		cout << setw(user_id_length) << "Owner Id" <<
			setw(user_group_length) << "Owner Group" <<
			setw(inode_id_length) << "Inode Id" <<
			setw(permission_length) << "O[RWE]G[RWE]E[RWE]";
	}

	cout << endl;


	for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
		if (strlen(directory.d_filename[i]) > 0) {
			Inode inode;
			Read_Inode(inode, directory.d_inodenumber[i]);

			char time[80];
			strftime(time, 80, "%Y-%m-%d %H:%M:%S", localtime(&inode.i_time));

			cout << setw(time_length) << time;
			cout << setw(type_length) << ((inode.i_mode & Inode::IDIRECTORY) ? "<DIR>" : "");
			cout << setw(size_length) << ((inode.i_mode & Inode::IDIRECTORY) ? " " : to_string(inode.i_size));
			cout << setw(file_name_length) << directory.d_filename[i];
			if (detail) {
				char tmp[16] = { bitset<9>(inode.i_permission).to_string()[0], bitset<9>(inode.i_permission).to_string()[1], bitset<9>(inode.i_permission).to_string()[2],' ',
				bitset<9>(inode.i_permission).to_string()[3], bitset<9>(inode.i_permission).to_string()[4], bitset<9>(inode.i_permission).to_string()[5],' ',
				bitset<9>(inode.i_permission).to_string()[6], bitset<9>(inode.i_permission).to_string()[7], bitset<9>(inode.i_permission).to_string()[8],'\0' };
				cout << setw(user_id_length) << inode.i_uid <<
					setw(user_group_length) << inode.i_gid <<
					setw(inode_id_length) << inode.i_number <<
					setw(permission_length) << tmp;
			}
			cout<< endl;

		}
	}

}

//根据文件名称打开一个当前目录的文件
File* Open_File(const char* file_name) 
{
	//检查文件名是否合法
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "文件名不合法" << endl;
		throw(ERROR_INVALID_FILENAME);
	}

	//查找文件是否存在
	Inode inode;
	int inode_num;
	bool exist = false;
	//检测当前目录是否有同名文件
	for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
		if (strcmp(directory.d_filename[i], file_name) == 0) {
			inode_num = directory.d_inodenumber[i];
			Read_Inode(inode, inode_num);
			if (inode.i_mode & Inode::IFILE) {
				exist = true;
			}
		}
	}
	if (exist == false) {
		cout << "文件名不合法，当前目录不存在名为" << file_name << "的文件" << endl;
		throw(ERROR_INVALID_FILENAME);
	}

	//引用计数++
	inode.i_count++;
	//访问时间修改
	inode.i_time = time(NULL);
	Write_Inode(inode, inode_num);

	File* file = new File;
	file->f_inodeid = inode_num;
	file->f_offset = inode.i_size;
	file->f_uid = user_id;

	return file;
}

//根据文件结构体关闭一个文件
void Close_File(File *file)
{
	Inode inode;
	int inode_num=file->f_inodeid;
	Read_Inode(inode, inode_num);

	//检查当前用户是不是文件的打开者
	if (user_id != file->f_uid) {
		cout << "非文件打开用户不可以对文件进行关闭操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//引用计数++
	inode.i_count--;
	//访问时间修改
	inode.i_time = time(NULL);
	Write_Inode(inode, inode_num);

	delete file;
}

//写文件
unsigned int Write_File(File* file, const char* content) 
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//检查当前用户是不是文件的打开者
	if (user_id != file->f_uid) {
		cout << "非文件打开用户不可以对文件进行写操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	//检查权限
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_W)) {
		cout << "当前用户没有权限对文件进行写操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_W)) {
		cout << "当前用户没有权限对文件进行写操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::ELSE_W)) {
		cout << "当前用户没有权限对文件进行写操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//检查文件大小是否已到达极限
	if (file->f_offset + strlen(content)+1 > BLOCK_SIZE * (6+2*128+2*128*128 - 2)) {
		cout << "超过文件的最大长度" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//当前要写入的blocknum
	unsigned int block_num_logical;
	unsigned int block_num_physical;
	unsigned int content_offset=0;

	while (content_offset < strlen(content)) {
		//如果当前块还未进行物理块的申请
		block_num_logical = (file->f_offset) / BLOCK_SIZE;
		if (file->f_offset == inode.i_size && (inode.i_size % BLOCK_SIZE == 0)) {
			//申请块
			if (block_num_logical <= 5) {
				Allocate_Block(block_num_physical);
				inode.i_addr[block_num_logical] = block_num_physical;
			}
			else if (block_num_logical >= 6 && block_num_logical <= 261) {
				if ((block_num_logical - 6) % 128 == 0) {
					Allocate_Block(block_num_physical);
					inode.i_addr[(block_num_logical - 6)/128+6] = block_num_physical;
				}
				unsigned int block_map_1[128];
				fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
				fd.seekg((inode.i_addr[(block_num_logical - 6) / 128 + 6]+BLOCK_POSITION)*BLOCK_SIZE, ios::beg);
				fd.read((char*)block_map_1, sizeof(block_map_1));
				fd.close();
				Allocate_Block(block_num_physical);
				block_map_1[(block_num_logical - 6) % 128] = block_num_physical;
				fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
				fd.seekg((inode.i_addr[(block_num_logical - 6) / 128 + 6] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
				fd.write((char*)block_map_1, sizeof(block_map_1));
				fd.close();
			}
			else if (block_num_logical >= 262) {
				unsigned int block_map_1[128], block_map_2[128];
				if ((block_num_logical - 262) % (128*128) == 0) {
					Allocate_Block(block_num_physical);
					inode.i_addr[(block_num_logical - 262) / 128/128 + 8] = block_num_physical;
				}
				if ((block_num_logical - 262) % 128 == 0) {
					Allocate_Block(block_num_physical);
					fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
					fd.seekg((inode.i_addr[(block_num_logical - 262) / 128 / 128 + 8] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
					fd.read((char*)block_map_1, sizeof(block_map_1));
					fd.close();
					block_map_1[(block_num_logical - 262) / 128 % 128] = block_num_physical;
					fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
					fd.seekg((inode.i_addr[(block_num_logical - 262) / 128 / 128 + 8] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
					fd.write((char*)block_map_1, sizeof(block_map_1));
					fd.close();
				}
				fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
				fd.seekg((inode.i_addr[(block_num_logical - 262) / 128 / 128 + 8] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
				fd.read((char*)block_map_1, sizeof(block_map_1));

				fd.seekg((block_map_1[(block_num_logical - 262) / 128 % 128] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
				fd.read((char*)block_map_2, sizeof(block_map_2));
				fd.close();
				Allocate_Block(block_num_physical);
				block_map_2[(block_num_logical - 262) % 128] = block_num_physical;
				fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
				fd.seekg((block_map_1[(block_num_logical - 262) / 128 % 128] + BLOCK_POSITION) * BLOCK_SIZE, ios::beg);
				fd.write((char*)block_map_2, sizeof(block_map_2));
				fd.close();
			}
		}
		unsigned int tmp2, tmp3;
		Get_Block_Pysical_Num(inode,block_num_logical, block_num_physical, tmp2, tmp3);
		//在当前块写入的位置
		unsigned int block_offset;
		block_offset = file->f_offset % BLOCK_SIZE;
		//本次需要写入的字节数
		unsigned int write_byte_num;
		write_byte_num = (strlen(content) - content_offset) >= (BLOCK_SIZE - block_offset) ? BLOCK_SIZE - block_offset : strlen(content) - content_offset;

		//从内存中读出该块内容
		char block_content[BLOCK_SIZE];
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.read(block_content, sizeof(block_content));
		fd.close();
		//将内容写入内存
		memcpy(block_content + block_offset, content+content_offset, write_byte_num);
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.write(block_content, sizeof(block_content));
		fd.close();

		content_offset += write_byte_num;
		file->f_offset += write_byte_num;
		if(file->f_offset> inode.i_size)
			inode.i_size= file->f_offset;
	}
	inode.i_time = time(NULL);

	Write_Inode(inode, inode_num);
	return strlen(content);
}

//更改文件指针
void Seek_File(File* file, unsigned int pos)
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//检查当前用户是不是文件的打开者
	if (user_id != file->f_uid) {
		cout << "非文件打开用户不可以对文件指针更改" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//检查权限
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_W)) {
		cout << "当前用户没有权限对文件进行指针更改" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_W)) {
		cout << "当前用户没有权限对文件进行指针更改" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::ELSE_W)) {
		cout << "当前用户没有权限对文件进行指针更改" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	if (inode.i_size < pos) {
		cout << "输入的文件指针位置大于文件长度，已定位到文件末尾" << endl;
	}
	file->f_offset = (inode.i_size < pos) ? inode.i_size : pos;
	

	Write_Inode(inode, inode_num);
}

//读文件
unsigned int Read_File(File* file,char* content)
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//检查当前用户是不是文件的打开者
	if (user_id != file->f_uid) {
		cout << "非文件打开用户不可以对文件进行读操作" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	//检查权限
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_R)) {
		cout << "当前用户没有权限读文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_R)) {
		cout << "当前用户没有权限读文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::ELSE_R)) {
		cout << "当前用户没有权限读文件" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	unsigned int block_num_logical;
	unsigned int block_num_physical;
	unsigned int content_offset=0;
	file->f_offset = 0;
	while (file->f_offset < inode.i_size) {
		block_num_logical = (file->f_offset) / BLOCK_SIZE;
		unsigned int tmp2, tmp3;
		Get_Block_Pysical_Num(inode,block_num_logical, block_num_physical, tmp2, tmp3);

		unsigned int read_byte_num;
		read_byte_num = ((inode.i_size - file->f_offset) < BLOCK_SIZE) ? inode.i_size - file->f_offset : BLOCK_SIZE;

		char block_content[BLOCK_SIZE];
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.read(block_content, sizeof(block_content));
		fd.close();

		memcpy(content + content_offset, block_content, read_byte_num);
		content_offset += read_byte_num;
		file->f_offset += read_byte_num;
	}
	//补一个尾零方便输出
	content[content_offset] = '\0';
	return strlen(content);
}

//更改一个文件的权限
void Edit_File_Permission(const char* directory_name, unsigned short permission,bool add)
{
	if (directory_name == NULL) {
		cout << "路径名不合法" << endl;
		throw(ERROR_INVALID_PATH);
	}
	char* reached_path = new char[strlen(directory_name) + 2];
	reached_path[0] = '\0';

	Directory current_directory = directory;
	unsigned int current_inode = 0;

	int index = 0;
	while (strlen(reached_path) < strlen(directory_name)) {
		char to_dir[FILE_NAME_MAX];
		for (int i = 0;; i++, index++) {
			if (directory_name[index] == '\\' || directory_name[index] == '/' || directory_name[index] == '\0') {
				to_dir[i] = '\0';
				index++;
				break;
			}
			to_dir[i] = directory_name[index];
		}
		bool find = false;
		//遍历目录
		for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
			if (strcmp(to_dir, current_directory.d_filename[i]) == 0) {
				Inode to_inode;
				current_inode = current_directory.d_inodenumber[i];
				Read_Inode(to_inode, current_inode);

				if (to_inode.i_mode == Inode::IDIRECTORY) {
					find = true;
					strcat(reached_path, to_dir);
					strcat(reached_path, "/");

					//更改current_directory
					fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
					fd.seekg(BLOCK_SIZE * (BLOCK_POSITION + to_inode.i_addr[0]), ios::beg);
					fd.read((char*)&current_directory, sizeof(current_directory));
					fd.close();
					break;
				}
				else {
					strcat(reached_path, to_dir);
					//找到文件
					if (strlen(reached_path) == strlen(directory_name)) {
						find = true;
					}
				}
			}
		}
		if (find == false) {
			cout << "目录不合法" << endl;
			throw(ERROR_INVALID_PATH);
		}
	}
	Inode file_inode;
	Read_Inode(file_inode, current_inode);
	if (add)
		file_inode.i_permission = file_inode.i_permission | permission;
	else 
		file_inode.i_permission = file_inode.i_permission & ~permission;
	
	Write_Inode(file_inode, current_inode);

	delete[] reached_path;
}
