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
	new_inode.i_time = time(NULL);
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
	Read_SuperBlock(superblock);
	superblock.s_finodenum--;
	Write_SuperBlock(superblock);
}

//ɾ��һ���ļ�
void Delete_File(const char* file_name)
{
	//����ļ����Ƿ�Ϸ�
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "�ļ������Ϸ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}
	//ȡ��superblock
	SuperBlock superblock;
	Read_SuperBlock(superblock);

	Inode inode;
	int inode_num;
	int file_index_in_directory;
	bool exist = false;
	//��⵱ǰĿ¼�Ƿ���ͬ���ļ�
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
		cout << "�ļ������Ϸ�����ǰĿ¼��������Ϊ" << file_name << "���ļ�" << endl;
		throw(ERROR_INVALID_FILENAME);
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
	else if (user_id != inode.i_uid&&user.u_gid[user_id] == inode.i_gid && !(directory_inode.i_permission & Inode::GROUP_W)) {
		cout << "��û��Ȩ��ɾ�����ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid)&& !(directory_inode.i_permission & Inode::OTHER_W)){
		cout << "��û��Ȩ��ɾ�����ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	//����ļ��Ƿ��ѱ��û���
	if (inode.i_count > 0) {
		cout << "��ǰ�ļ��ѱ��򿪣��޷�ɾ��" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	
	//��ʼ����ɾ��
	//�����ͷ����е�block
	while (inode.i_size > 0) {
		unsigned int block_num = (inode.i_size-1) / BLOCK_SIZE;
		inode.i_size-= BLOCK_SIZE;
		Free_Block(block_num);
	}
	//����λʾͼ���ͷ�Inode��
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);
	inode_bitmap[inode.i_number] = 0;
	Write_InodeBitMap(inode_bitmap);

	//����Ŀ¼
	directory.d_filename[file_index_in_directory][0] = '\0';
	directory.d_inodenumber[file_index_in_directory] = -1;
	fd.seekg((BLOCK_POSITION + directory_inode.i_addr[0]) * BLOCK_SIZE, ios::beg);
	fd.write((char*)&directory, sizeof(directory));

	//���³�����
	superblock.s_finodenum++;
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
		setw(size_length) << "File Size(B)" <<
		setw(permission_length) << "Permission" <<
		setw(time_length) << "Edit Time" << endl;

	for (int i = 0; i < SUBDIRECTORY_NUM; i++) {
		if (strlen(directory.d_filename[i]) > 0) {
			Inode inode;
			Read_Inode(inode, directory.d_inodenumber[i]);

			char time[80];
			strftime(time, 80, "%Y-%m-%d %H:%M:%S", localtime(&inode.i_time));

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

//�����ļ����ƴ�һ����ǰĿ¼���ļ�
File* Open_File(const char* file_name) 
{
	//����ļ����Ƿ�Ϸ�
	if (file_name == NULL || strlen(file_name) > FILENAME_MAX) {
		cout << "�ļ������Ϸ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}

	//�����ļ��Ƿ����
	Inode inode;
	int inode_num;
	bool exist = false;
	//��⵱ǰĿ¼�Ƿ���ͬ���ļ�
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
		cout << "�ļ������Ϸ�����ǰĿ¼��������Ϊ" << file_name << "���ļ�" << endl;
		throw(ERROR_INVALID_FILENAME);
	}

	//���ü���++
	inode.i_count++;
	//����ʱ���޸�
	inode.i_time = time(NULL);
	Write_Inode(inode, inode_num);

	File* file = new File;
	file->f_inodeid = inode_num;
	file->f_offset = inode.i_size;
	file->f_uid = user_id;

	return file;
}

//�����ļ��ṹ��ر�һ���ļ�
void Close_File(File *file)
{
	Inode inode;
	int inode_num=file->f_inodeid;
	Read_Inode(inode, inode_num);

	//��鵱ǰ�û��ǲ����ļ��Ĵ���
	if (user_id != file->f_uid) {
		cout << "���ļ����û������Զ��ļ����йرղ���" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//���ü���++
	inode.i_count--;
	//����ʱ���޸�
	inode.i_time = time(NULL);
	Write_Inode(inode, inode_num);

	delete file;
}

//д�ļ�
unsigned int Write_File(File* file, const char* content) 
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//��鵱ǰ�û��ǲ����ļ��Ĵ���
	if (user_id != file->f_uid) {
		cout << "���ļ����û������Զ��ļ�����д����" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//���Ȩ��
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����д����" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����д����" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::OTHER_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����д����" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//����ļ���С�Ƿ��ѵ��Ｋ��
	if (file->f_offset + strlen(content)+1 > BLOCK_SIZE * (NINODE - 2)) {
		cout << "�����ļ�����󳤶�" << endl;
		throw(ERROR_OUT_OF_SPACE);
	}

	//��ǰҪд���blocknum
	unsigned int block_num_logical;
	unsigned int block_num_physical;
	unsigned int content_offset=0;

	while (content_offset < strlen(content)) {
		//�����ǰ�黹δ��������������
		block_num_logical = (file->f_offset) / BLOCK_SIZE;
		if (file->f_offset == inode.i_size && (inode.i_size % BLOCK_SIZE == 0)) {
			Allocate_Block(block_num_physical);

			inode.i_addr[block_num_logical] = block_num_physical;
		}
		block_num_physical = inode.i_addr[block_num_logical];
		//�ڵ�ǰ��д���λ��
		unsigned int block_offset;
		block_offset = file->f_offset % BLOCK_SIZE;
		//������Ҫд����ֽ���
		unsigned int write_byte_num;
		write_byte_num = (strlen(content) - content_offset) >= (BLOCK_SIZE - block_offset) ? BLOCK_SIZE - block_offset : strlen(content) - content_offset;

		//���ڴ��ж����ÿ�����
		char block_content[BLOCK_SIZE];
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.read(block_content, sizeof(block_content));

		//������д���ڴ�
		memcpy(block_content + block_offset, content+content_offset, write_byte_num);
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.write(block_content, sizeof(block_content));
		
		content_offset += write_byte_num;
		file->f_offset += write_byte_num;
		if(file->f_offset> inode.i_size)
			inode.i_size= file->f_offset;
	}
	inode.i_time = time(NULL);

	Write_Inode(inode, inode_num);
	return strlen(content);
}

//�����ļ�ָ��
void Seek_File(File* file, unsigned int pos)
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//��鵱ǰ�û��ǲ����ļ��Ĵ���
	if (user_id != file->f_uid) {
		cout << "���ļ����û������Զ��ļ�ָ�����" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	//���Ȩ��
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����ָ�����" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����ָ�����" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::OTHER_W)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�����ָ�����" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	if (inode.i_size < pos) {
		cout << "������ļ�ָ��λ�ô����ļ����ȣ��Ѷ�λ���ļ�ĩβ" << endl;
	}
	file->f_offset = (inode.i_size < pos) ? inode.i_size : pos;
	

	Write_Inode(inode, inode_num);
}

//���ļ�
unsigned int Read_File(File* file,char* content)
{
	Inode inode;
	int inode_num = file->f_inodeid;
	Read_Inode(inode, inode_num);
	User user;
	Read_User(user);

	//��鵱ǰ�û��ǲ����ļ��Ĵ���
	if (user_id != file->f_uid) {
		cout << "���ļ����û������Զ��ļ����ж�����" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	//���Ȩ��
	if (user_id == inode.i_uid && !(inode.i_permission & Inode::OWNER_R)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if (user_id != inode.i_uid && user.u_gid[user_id] == inode.i_gid && !(inode.i_permission & Inode::GROUP_R)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}
	else if ((user_id != inode.i_uid && user.u_gid[user_id] != inode.i_gid) && !(inode.i_permission & Inode::OTHER_R)) {
		cout << "��ǰ�û�û��Ȩ�޶��ļ�" << endl;
		throw(ERROR_NO_PERMISSION);
	}

	unsigned int block_num_logical;
	unsigned int block_num_physical;
	unsigned int content_offset=0;
	file->f_offset = 0;
	while (file->f_offset < inode.i_size) {
		block_num_logical = (file->f_offset) / BLOCK_SIZE;
		block_num_physical = inode.i_addr[block_num_logical];

		unsigned int read_byte_num;
		read_byte_num = ((inode.i_size - file->f_offset) < BLOCK_SIZE) ? inode.i_size - file->f_offset : BLOCK_SIZE;

		char block_content[BLOCK_SIZE];
		fd.seekg(BLOCK_SIZE * (BLOCK_NUM + block_num_physical), ios::beg);
		fd.read(block_content, sizeof(block_content));


		memcpy(content + content_offset, block_content, read_byte_num);
		content_offset += read_byte_num;
		file->f_offset += read_byte_num;
	}
	//��һ��β�㷽�����
	content[content_offset] = '\0';
	return strlen(content);
}
