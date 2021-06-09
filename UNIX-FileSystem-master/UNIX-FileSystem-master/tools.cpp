#include "head.h"
#include "error.h"

//��ʼ�������ļ�ϵͳ�ռ�
void Init()
{
	//�ڵ�ǰĿ�½��ļ���Ϊ�ļ���
	fstream fd(DISK_NAME, ios::out );
	fd.close();
	fd.open(DISK_NAME, ios::out|ios::in|ios::binary);

	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//��SuperBlock���г�ʼ��
	SuperBlock superblock; 
	superblock.s_inodenum = INODE_NUM;
	//�ܿ��� = Block�� + SuperBlock��ռ���� + Inodeλͼ��ռ����+ Inode��ռ����
	superblock.s_blocknum = BLOCK_NUM+ BLOCK_POSITION;
	superblock.s_finodenum = INODE_NUM - 2;//�������Ѿ�����
	superblock.s_fblocknum = BLOCK_NUM - 2;//�������Ѿ�����
	//ֱ�ӹ����Block�ռ䣨0-49��
	for (int i = 0; i < FREE_BLOCK_GROUP_NUM; i++) 
		superblock.s_free[i] = FREE_BLOCK_GROUP_NUM - 1 - i;
	superblock.s_nfree = FREE_BLOCK_GROUP_NUM - 1 - 2;//�������Ѿ�����
	//д���ڴ�
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
	
	//���г�������
	unsigned int stack[FREE_BLOCK_GROUP_NUM];
	for (int i = 2; i <= BLOCK_NUM / FREE_BLOCK_GROUP_NUM;i++) {
		for (unsigned j = 0; j < FREE_BLOCK_GROUP_NUM; j++) {
			stack[j] = FREE_BLOCK_GROUP_NUM * i - 1 - j;
		}
		if(i== BLOCK_NUM / FREE_BLOCK_GROUP_NUM)
			stack[0] = 0;
		//д���ڴ�
		if (i != BLOCK_NUM / FREE_BLOCK_GROUP_NUM)
			fd.seekg((BLOCK_POSITION+ stack[0]- FREE_BLOCK_GROUP_NUM) * BLOCK_SIZE, ios::beg);
		else
			fd.seekg((BLOCK_POSITION + BLOCK_NUM - FREE_BLOCK_GROUP_NUM-1) * BLOCK_SIZE, ios::beg);
		fd.write((char*)&stack, sizeof(stack));
	}

	//��ʼ��λͼ����ʼ��ǰ����Ϊ1��ʣ�µ��ڶ���ʱ�Ѿ�����Ϊ0��
	unsigned int inode_bitmap[INODE_NUM] = { 0 };
	inode_bitmap[0] = 1;
	inode_bitmap[1] = 1;
	//д���ڴ�
	//д���ڴ�
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);

	//������Ŀ¼
	Inode Inode_root;
	Inode_root.i_number=0;//Inode�ı��
	Inode_root.i_addr[0]=0;//��Ӧ0��Block
	Inode_root.i_mode=Inode::IDIRECTORY;//Ŀ¼
	Inode_root.i_count=0;//���ü���
	Inode_root.i_uid=0;//����Ա
	Inode_root.i_gid=1;//�ļ������ߵ����ʶ
	Inode_root.i_size=0;//Ŀ¼��СΪ0
	Inode_root.i_time= time(NULL);//������ʱ��
	Inode_root.i_permission = 0777;
	//д���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&Inode_root, sizeof(Inode_root));
	//0��Blockд��Directory
	Directory root_directory;
	strcpy(root_directory.d_filename[0],".");//0���Լ�
	root_directory.d_inodenumber[0] = 0;
	strcpy(root_directory.d_filename[1], "..");//1�Ǹ��ף��˴������Լ���
	root_directory.d_inodenumber[1] = 0;
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		root_directory.d_filename[i][0] = '\0';
	}
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		root_directory.d_inodenumber[i] = -1;
	}
	fd.seekg(BLOCK_POSITION*BLOCK_SIZE, ios::beg);
	fd.write((char*)&root_directory, sizeof(root_directory));

	//�����û��ļ�
	Inode Inode_accounting;
	Inode_accounting.i_number = 1;//Inode�ı��
	Inode_accounting.i_addr[0] = 1;//��Ӧ1��Block
	Inode_accounting.i_mode = Inode::IFILE;//�ļ�
	Inode_accounting.i_count = 0;//���ü���
	Inode_accounting.i_permission = 0700;//����Ա�ɶ�д
	Inode_accounting.i_uid = 0;//����Ա
	Inode_accounting.i_gid = 1;//�ļ������ߵ����ʶ
	Inode_accounting.i_size = 0;//Ŀ¼��СΪ1
	Inode_accounting.i_time = time(NULL);//������ʱ��
	//д���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE+INODE_SIZE, ios::beg);
	fd.write((char*)&Inode_accounting, sizeof(Inode_accounting));

	//���������˻�
	User user;
	strcpy(user.u_name[0], "root");
	strcpy(user.u_password[0], "root");
	strcpy(user.u_name[1], "juju");
	strcpy(user.u_password[1], "juju");
	user.u_id[0] = 0;
	user.u_id[1] = 1;
	for (int i = 2; i < USER_NUM; i++) {
		user.u_id[i] = -1;
	}
	for (int i = 2; i < USER_NUM; i++) {
		user.u_name[i][0] = '\0';
	}
	for (int i = 2; i < USER_NUM; i++) {
		user.u_password[i][0] = '\0';
	}
	user.u_gid[0] = 1;
	user.u_gid[1] = 2;
	//д���ڴ�
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE + Inode_accounting.i_addr[0]*BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
	
	//�����Ŀ¼
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&directory, sizeof(directory));
	
	fd.close();

	//��root�û������ļ�Ŀ¼
	user_id = 0;
	Create_Directory("bin");
	Create_Directory("etc");
	Create_Directory("home");
	Create_Directory("dev");
	Open_Directory("./home");
	Create_Directory("texts");
	Create_Directory("reports");
	Create_Directory("photos");
	Open_Directory("./texts");
	Create_File("test.txt");
	
}

//���ļ�ϵͳ
void Activate()
{
	//���ļ���
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//���û�д��ļ��������ʾ��Ϣ��throw����
	if (!fd.is_open()) {
		cout << "�޷����ļ���myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//��ȡ��ǰĿ¼
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&directory, sizeof(directory));

	//������inode�Ĵ�������
	unsigned int inode_bitmap[INODE_NUM];
	Read_InodeBitMap(inode_bitmap);
	for (int i = 0; i < INODE_NUM; i++) {
		if (inode_bitmap[i] == 1) {
			Inode inode;
			Read_Inode(inode, i);
			inode.i_count = 0;
			Write_Inode(inode, i);
		}
	}

	fd.close();

	user_id = -1;
}

void help()
{
	cout << "�й�ĳ���������ϸ��Ϣ������� HELP ������" << endl;
	cout << "HELP           �����ĵ���" << endl;
	cout << "ATTRIB         ��ʾ������ļ����ԡ�" << endl;
	cout << "CD             ��ʾ��ǰĿ¼�����ƻ�����ġ�" << endl;
	cout << "DEL            ɾ������һ���ļ���" << endl;
	cout << "DIR            ��ʾһ��Ŀ¼�е��ļ�����Ŀ¼��" << endl;
	cout << "EXIT           �˳��ļ�ϵͳ��" << endl;
	cout << "MKDIR          ����һ��Ŀ¼��" << endl;
	cout << "RMDIR          ɾ��Ŀ¼��" << endl;
	cout << "PRINT          ��ӡ�ļ����ݡ�" << endl;
	cout << "WRITE          ���ļ���д������" << endl;
	cout << "OPEN           ��һ���ļ�" << endl;
	cout << "CLOSE          �ر�һ���ļ�" << endl;
	cout << "CREATE         ����һ���ļ�" << endl;
	cout << "OPENLIST       ��ǰ���ļ��б�" << endl;
	cout << "FSEEK          ����һ���ļ���ָ��" << endl;
	cout << "LOGOUT         �û��˳���¼" << endl;
	cout << "WHOAMI         ��ʾ��ǰ�û���Ϣ" << endl;
	cout << "FORMAT         ��ʽ���ļ���" << endl;
	cout << "REGISTER       �û�ע��" << endl;
	cout << "DELETEACCOUNT  ɾ���û���root�û��£�" << endl;
	cout << "SU             �ı��û�" << endl;
	cout << "CHGRP          �ı��û������飨root�û��£�" << endl;
	cout << "USERLIST       ��ʾ�����û���Ϣ��root�û��£�" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "" << endl;

}

void help_attrib()
{
	cout << "�����ļ��Ķ�д����" << endl;
	cout << endl;
	cout << "ATTRIB [+R | -R] [+W | -W] [+E | -E] [O | G | E] [path][filename]" << endl;
	cout << endl;
	cout << "  +   �������ԡ�" << endl;
	cout << "  -   ������ԡ�" << endl;
	cout << "  R   ���ļ����ԡ�" << endl;
	cout << "  W   д�ļ����ԡ�" << endl;
	cout << "  E   ִ���ļ����ԡ�" << endl;
	cout << "  /O  �ļ��������û���" << endl;
	cout << "  /G  �ļ�ͬ�����û���" << endl;
	cout << "  /E  ���ļ������ߡ����ļ�ͬ�����û�" << endl;
	cout << "[path][filename]" << endl;
	cout << "      ָ������Ҫ������ļ���" << endl;
}

void help_cd()
{
	cout << "��ʾ��ǰĿ¼����ı䵱ǰĿ¼��" << endl;
	cout << endl;
	cout << "CD [path]" << endl;
	cout << endl;
	cout << "��������ֻ���� CD������ʾ��ǰ��������Ŀ¼��" << endl;
	cout << "path����.����Ŀ¼��..����Ŀ¼��·������/��\\�ָ�" << endl;
}
void help_del()
{
	cout << "ɾ��һ�������ļ���" << endl;
	cout << endl;
	cout << "DEL names" << endl;
	cout << endl;
	cout << "ɾ�������һ���ļ���" << endl;
	cout << "���û�δ�ر��ļ�ʱ����ɾ���ļ���" << endl;
}
void help_dir()
{
	cout << "��ʾĿ¼�е��ļ�����Ŀ¼�б�" << endl;
	cout << endl;
	cout << "DIR [/Q]" << endl;
	cout << endl;
	cout << "  /Q  ��ʾ��ϸ��Ϣ��" << endl;
}
void help_exit()
{
	cout << "�˳�����(�ļ�ϵͳ)��" << endl;
	cout << endl;
	cout << "DIR [exitCode]" << endl;
	cout << endl;
	cout << "  exitCode    ָ��һ�����ֺ��롣����˳��������Ǹ��������ù����˳����롣" << endl;
}
void help_mkdir()
{
	cout << "����Ŀ¼��" << endl;
	cout << endl;
	cout << "MKDIR dir" << endl;
	cout << endl;
	cout << "  dir    ������Ŀ¼���ơ�" << endl;
}
void help_rmdir()
{
	cout << "ɾ��һ��Ŀ¼��" << endl;
	cout << endl;
	cout << "MKDIR dir" << endl;
	cout << endl;
	cout << "  dir    ��ɾ����Ŀ¼���ƣ����п�Ŀ¼���Ա�ɾ������" << endl;
}
void help_print()
{
	cout << "��ӡ�ı��ļ���" << endl;
	cout << endl;
	cout << "PRINT [filename] [path]" << endl;
	cout << endl;
	cout << "  filename ��ӡ���ļ����ơ�" << endl;
	cout << "  path     ��ѡ����ӡ���ļ�ϵͳ����Ŀ¼���ļ����ơ�" << endl;
}
void help_write()
{
	cout << "��ӡ�ı��ļ���" << endl;
	cout << endl;
	cout << "PRINT [filename] [-s|-f] [path|content]" << endl;
	cout << endl;
	cout << "  filename д����ļ����ơ�" << endl;
	cout << "  -s      ����Ļд�롣" << endl;
	cout << "  -f      ���ļ�д�롣" << endl;
	cout << "  path    д�뵽�ļ�ϵͳ����Ŀ¼���ļ����ơ���" << endl;
	cout << "  content д���ļ����ݡ�" << endl;
}
void help_open()
{
	cout << "���ļ���" << endl;
	cout << endl;
	cout << "OPEN [filename]" << endl;
	cout << endl;
	cout << "  filename �򿪵��ļ����ơ�" << endl;
}
void help_close()
{
	cout << "�ر��ļ���" << endl;
	cout << endl;
	cout << "CLOSE [filename]" << endl;
	cout << endl;
	cout << "  filename �رյ��ļ����ơ�" << endl;
}
void help_fseek()
{
	cout << "�����ļ�ָ�롣" << endl;
	cout << endl;
	cout << "FSEEK filename pos" << endl;
	cout << endl;
	cout << "  filename �ļ����ơ�" << endl;
	cout << "  pos      �ļ�ָ�붨λλ�á�" << endl;
}
void help_create()
{
	cout << "�����ļ���" << endl;
	cout << endl;
	cout << "CREATE filename" << endl;
	cout << endl;
	cout << "  filename �ļ����ơ�" << endl;
}
void help_logout()
{
	cout << "�û��ǳ���" << endl;
	cout << endl;
	cout << "LOGOUT" << endl;
}
void help_whoami()
{
	cout << "��ȡ��ǰ��¼�û���" << endl;
	cout << endl;
	cout << "WHOAMI" << endl;
}
void help_format()
{
	cout << "��ʽ���ļ���" << endl;
	cout << endl;
	cout << "FORMAT" << endl;
}
void help_register()
{
	cout << "�������û���" << endl;
	cout << endl;
	cout << "REGISTER username password" << endl;
	cout << "  username �û�����" << endl;
	cout << "  password �û����롣" << endl;
}
void help_deleteaccount()
{
	cout << "ɾ���û���" << endl;
	cout << endl;
	cout << "DELETEACCOUNT username" << endl;
	cout << "  username �û�����" << endl;
}
void help_su()
{
	cout << "�ı��û�" << endl;
	cout << endl;
	cout << "SU username password" << endl;
	cout << "  username �û�����" << endl;
	cout << "  password �û����롣" << endl;
}
void help_chgrp()
{
	cout << "�ı��û�������" << endl;
	cout << endl;
	cout << "CHGRP username usergroupid" << endl;
	cout << "  username    �û�����" << endl;
	cout << "  usergroupid �û������顣" << endl;
}
void help_userlist()
{
	cout << "��ʾ�����û���Ϣ" << endl;
	cout << endl;
	cout << "USERLIST" << endl;
}
void help_openlist()
{
	cout << "��ʾ���д��ļ��б�" << endl;
	cout << endl;
	cout << "OPENLIST" << endl;
}