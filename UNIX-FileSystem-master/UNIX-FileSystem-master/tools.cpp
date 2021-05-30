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
	superblock.s_inodesize = INODE_SIZE;
	superblock.s_blocksize = BLOCK_SIZE;
	superblock.s_finodenum = INODE_NUM - 2;//�������Ѿ�����
	superblock.s_fblocknum = BLOCK_NUM - 2;//�������Ѿ�����
	//ֱ�ӹ����Block�ռ䣨0-49��
	for (int i = 0; i < FREE_BLOCK_GROUP_NUM; i++) 
		superblock.s_free[i] = FREE_BLOCK_GROUP_NUM - 1 - i;
	superblock.s_nfree = FREE_BLOCK_GROUP_NUM - 1 - 2;//�������Ѿ�����
	//д���ڴ�
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
	
	//���г������ӣ���֤500/50������
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

	//������Ŀ¼��Ŀ¼��Ȩ����ֵ��
	Inode Inode_root;
	Inode_root.i_number=0;//Inode�ı��
	Inode_root.i_addr[0]=0;//��Ӧ0��Block
	Inode_root.i_mode=Inode::IDIRECTORY;//Ŀ¼
	Inode_root.i_count=0;//���ü���
	Inode_root.i_uid=0;//����Ա
	Inode_root.i_gid=1;//�ļ������ߵ����ʶ
	Inode_root.i_size=1;//Ŀ¼��СΪ1
	Inode_root.time= time(NULL);//������ʱ��
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
	fd.seekg(BLOCK_POSITION*BLOCK_SIZE, ios::beg);
	fd.write((char*)&root_directory, sizeof(root_directory));

	//�����û��ļ�
	Inode Inode_accounting;
	Inode_accounting.i_number = 1;//Inode�ı��
	Inode_accounting.i_addr[0] = 1;//��Ӧ1��Block
	Inode_root.i_mode = Inode::IFILE;//�ļ�
	Inode_root.i_count = 0;//���ü���
	Inode_root.i_permission = 0700;//����Ա�ɶ�д
	Inode_root.i_uid = 0;//����Ա
	Inode_root.i_gid = 1;//�ļ������ߵ����ʶ
	Inode_root.i_size = 1;//Ŀ¼��СΪ1
	Inode_root.time = time(NULL);//������ʱ��
	//д���ڴ�
	fd.seekg(INODE_POSITION * BLOCK_SIZE+INODE_SIZE, ios::beg);
	fd.write((char*)&Inode_accounting, sizeof(Inode_accounting));

	//���������˻�
	User user;
	strcpy(user.u_name[0], "root");
	strcpy(user.u_password[0], "root");
	strcpy(user.u_name[1], "juju");
	strcpy(user.u_password[1], "juju");
	for (int i = 0; i < USER_NUM; i++) {//0 �������Ա
		user.u_id[i] = i;
	}
	user.u_gid[0] = 1;
	user.u_gid[1] = 2;
	//д���ڴ�
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE + Inode_accounting.i_addr[0]*BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));

	fd.close();
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

}