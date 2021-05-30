//**********************�й��ļ��Ĳ����ĺ���***********************
#include "head.h"
#include "error.h"


//����һ���ļ�
void Create_File(const char* file_name)
{
	//����ļ����Ƿ�Ϸ�
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "�ļ������Ϸ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//ȡ��superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	//�Ƿ��п��е�block��inode
	if (superblock.s_fblocknum <= 0 || superblock.s_finodenum <= 0)
	{
		cout << "�ռ䲻�㣬�޷��������ļ�" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//��⵱ǰĿ¼�Ƿ���ͬ���ļ�
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
				cout << "�ļ������Ϸ�����ǰĿ¼�Ѵ�����Ϊ" << file_name << "���ļ�" << endl;
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
		cout << "��ǰĿ¼�Ѵ�" << SUBDIRECTORY_NUM << "���������ƣ��޷����������ļ�" << endl;
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

	//����User�ṹ��
	User user;
	Read_User(user);
	//�����µ�Inode
	Inode new_inode;
	new_inode.i_number = new_inode_index;//Inode�ı��
	new_inode.i_mode = Inode::IFILE;//�ļ�
	new_inode.i_size = 0;
	new_inode.i_uid = user_id;
	new_inode.i_gid = user.u_gid[user_id];
	new_inode.i_permission = 0777;
	new_inode.i_count = 0;
	new_inode.time = time(NULL);
	//д��Inode���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE + new_inode.i_number * INODE_SIZE, ios::beg);
	fd.write((char*)&new_inode, sizeof(new_inode));

	//����λʾͼ
	inode_bitmap[new_inode.i_number] = 1;
	Write_InodeBitMap(inode_bitmap);

	//����Ŀ¼
	int directory_inode_index = directory.d_inodenumber[0];//"."
	Inode directory_inode;
	Read_Inode(directory_inode, directory_inode_index);
	//���뵱ǰĿ¼
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

	//���³�����
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);
}

//չʾ�ļ��б�
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