//**********************�й��ļ��Ĳ����ĺ���***********************
#include "head.h"
#include "error.h"


//����һ��Ŀ¼
void Create_Directory(const char* directory_name)
{
	//���Ŀ¼���Ƿ�Ϸ�
	if (directory_name == NULL || strlen(directory_name) > FILENAME_MAX) {
		cout << "Ŀ¼�����Ϸ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//ȡ��superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	//�Ƿ��п��е�block��inode
	if (superblock.s_fblocknum <= 0 || superblock.s_finodenum <= 0)
	{
		cout << "�ռ䲻�㣬�޷�����Ŀ¼" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//��⵱ǰĿ¼�Ƿ���ͬ��Ŀ¼
	for (int i = 0; i < SUBDIRECTORY_NUM; i++)
	{
		if (strcmp(directory.d_filename[i], directory_name) == 0)
		{
			Inode tmp_inode;
			int tmp_inode_number = directory.d_inodenumber[i];

			fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
			fd.seekg(INODE_POSITION * BLOCK_SIZE + tmp_inode_number * INODE_SIZE, ios::beg);
			fd.read((char*)&tmp_inode, sizeof(tmp_inode));
			fd.close();
			if (tmp_inode.i_mode & Inode::IFILE)
				continue;
			else {
				cout << "�ļ������Ϸ�����ǰĿ¼�Ѵ�����Ϊ" << directory_name << "���ļ�" << endl;
				throw(ERROR_INVALID_FILENAME);
			}
		}
	}

	//��⵱ǰĿ¼���ļ������Ƿ�ﵽ����
	int itemCounter = 0;
	for (int i = 0; i < SUBDIRECTORY_NUM; i++)
		if (strlen(directory.d_filename[i]) > 0)
			itemCounter++;

	if (itemCounter == SUBDIRECTORY_NUM)
	{
		cout << "��ǰĿ¼�Ѵ�" << SUBDIRECTORY_NUM << "���������ƣ��޷���������Ŀ¼" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//Ѱ���µ�inode
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);

	int new_inode_index = 0;
	unsigned int new_block_addr = -1;
	for (; new_inode_index < INODE_NUM; new_inode_index++)
		if (inode_bitmap[new_inode_index] == 0)
			break;

	//Ѱ���µ�block
	unsigned int block_num;
	Allocate_Block(block_num);

	//����User�ṹ��
	User user;
	Read_User(user);
	//�����µ�Inode
	Inode new_inode;
	new_inode.i_number = new_inode_index;//Inode�ı��
	new_inode.i_mode = Inode::IDIRECTORY;//�ļ�
	new_inode.i_size = 0;
	new_inode.i_uid = user_id;
	new_inode.i_gid = user.u_gid[user_id];
	new_inode.i_addr[0] = block_num;
	new_inode.i_permission = 0777;
	new_inode.i_count = 0;
	new_inode.i_time = time(NULL);
	//д��Inode���ڴ�
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));
	fd.close();

	//����λʾͼ
	inode_bitmap[new_inode.i_number] = 1;
	Write_InodeBitMap(inode_bitmap);

	//��directoryд�뵽block
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
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((block_num+BLOCK_POSITION)*BLOCK_SIZE,ios::beg);
	fd.write((char*)&new_directory, sizeof(new_directory));
	fd.close();

	//����Ŀ¼
	int directory_inode_index = directory.d_inodenumber[0];//"."
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_index);
	//���뵱ǰĿ¼
	for (int i = 2; i < SUBDIRECTORY_NUM; i++)
	{
		if (strlen(directory.d_filename[i]) == 0)
		{
			strcat(directory.d_filename[i], directory_name);
			directory.d_inodenumber[i] = new_inode.i_number;
			break;
		}
	}
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));
	fd.close();

	//���³�����
	Read_SuperBlock(superblock);
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);

}

//ɾ��һ��Ŀ¼
void Remove_Directory(const char* directory_name)
{
	//����ļ����Ƿ�Ϸ�
	if (directory_name == NULL || strlen(directory_name) > FILENAME_MAX|| strcmp(directory_name, ".")==0|| strcmp(directory_name, "..") == 0) {
		cout << "Ŀ¼�����Ϸ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//ȡ��superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	Inode inode;
	int inode_num;
	int directory_index_in_directory;
	bool exist = false;
	//��⵱ǰĿ¼�Ƿ���ͬ���ļ�
	for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
		if (strcmp(directory.d_filename[i], directory_name) == 0) {
			inode_num = directory.d_inodenumber[i];
			Read_Inode(inode, inode_num);
			if (inode.i_mode & Inode::IDIRECTORY) {
				exist = true;
				directory_index_in_directory = i;
			}
		}
	}
	if (exist == false) {
		cout << "�ļ������Ϸ�����ǰĿ¼��������Ϊ" << directory_name << "��Ŀ¼" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//���Ŀ¼�Ƿ�Ϊ��
	Directory delete_directory;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((BLOCK_POSITION + inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.read((char*)&delete_directory, sizeof(delete_directory));
	fd.close();

	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		if (strlen(delete_directory.d_filename[i]) != 0) {
			cout << "��ǰĿ¼��Ϊ�գ�������ɾ��" << endl;
			throw(ERROR_NO_PERMISSION);
		}
	}

	//���ɾ����Ȩ�ޣ�ɾ����Ҫ��һ��Ŀ¼��дȨ�ޣ�
	//���Ȳ�����һ��Ŀ¼
	//����User�ṹ��
	User user;
	Read_User(user);
	int directory_inode_num = directory.d_inodenumber[0];
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_num);
	if (user_id == inode.i_uid && !(directory_inode.i_permission & Inode::OWNER_W)) {
		cout << "��û��Ȩ��ɾ�����ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(directory_inode.i_permission & Inode::GROUP_W)) {
		cout << "��û��Ȩ��ɾ�����ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(directory_inode.i_permission & Inode::ELSE_W)) {
		cout << "��û��Ȩ��ɾ�����ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	
	//��ʼ����ɾ��
	//�����ͷ�block
	Free_Block(inode.i_addr[0]);
	//����λʾͼ���ͷ�Inode��
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);
	inode_bitmap[inode.i_number] = 0;
	Write_InodeBitMap(inode_bitmap);

	//����Ŀ¼
	directory.d_filename[directory_index_in_directory][0] = '\0';
	directory.d_inodenumber[directory_index_in_directory] = -1;
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));
	fd.close();

	//���³�����
	superblock.s_finodenum++;
	Write_SuperBlock(superblock);
}



//��һ��Ŀ¼
void Open_Directory(const char* directory_name)
{
	if (directory_name == NULL) {
		cout << "·�������Ϸ�" << endl;
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
		//����Ŀ¼
		for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
			if (strcmp(to_dir, directory.d_filename[i]) == 0) {
				Inode to_inode;
				Read_Inode(to_inode, directory.d_inodenumber[i]);

				if (to_inode.i_mode == Inode::IDIRECTORY) {
					find = true;
					strcat(reached_path, to_dir);
					strcat(reached_path, "/");

					//����directory
					fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
					fd.seekg(BLOCK_SIZE * (BLOCK_POSITION + to_inode.i_addr[0]), ios::beg);
					fd.read((char*)&directory, sizeof(directory));
					fd.close();
					break;
				}
			}
		}
		if (find == false) {
			cout << "Ŀ¼ " << to_dir << "�����ڣ�" ;
			if (strlen(reached_path))
				cout << "Ŀǰ�ѽ���·����" << reached_path<<endl;
			else
				cout << "·��δ�ı�" << endl;
			throw(ERROR_INVALID_PATH);
		}
	}

	delete [] reached_path;
}

//��ȡ��ǰĿ¼����ʵ������˵Ӧ�÷���char*��
string Current_Directory()
{
	//��ȡ��ǰĿ¼
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&directory, sizeof(directory));
	fd.close();

	Directory father_directory,current_directory= directory;
	
	string dir_path = "";
	while (current_directory.d_inodenumber[0] != current_directory.d_inodenumber[1]) {
		Inode father_inode;
		Read_Inode(father_inode, current_directory.d_inodenumber[1]);
		fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
		fd.seekg((BLOCK_POSITION + father_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
		fd.read((char*)&father_directory, sizeof(father_directory));
		fd.close();
		for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
			if (father_directory.d_inodenumber[i]== current_directory.d_inodenumber[0]) {
				dir_path = father_directory.d_filename[i] +("\\"+ dir_path);
				current_directory = father_directory;
			}
		}
	}
	dir_path = "root\\" + dir_path;
	dir_path = dir_path.substr(0, dir_path.length() - 1);
	return dir_path;
}

