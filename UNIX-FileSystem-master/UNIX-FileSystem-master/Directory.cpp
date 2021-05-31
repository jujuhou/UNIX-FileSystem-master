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

			fd.seekg(INODE_POSITION * BLOCK_SIZE + tmp_inode_number * INODE_SIZE, ios::beg);
			fd.read((char*)&tmp_inode, sizeof(tmp_inode));
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
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));

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
	fd.seekg((block_num+BLOCK_POSITION)*BLOCK_SIZE,ios::beg);
	fd.write((char*)&new_directory, sizeof(new_directory));

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
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));

	//���³�����
	Read_SuperBlock(superblock);
	superblock.s_finodenum--;
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
					fd.seekg(BLOCK_SIZE * (BLOCK_POSITION + to_inode.i_addr[0]), ios::beg);
					fd.read((char*)&directory, sizeof(directory));
					break;
				}
			}
		}
		if (find == false) {
			cout << "Ŀǰ�ѽ���·����" << reached_path << "��Ŀ¼ " << to_dir << "������" << endl;
			throw(ERROR_INVALID_PATH);
		}
	}

	delete [] reached_path;
}