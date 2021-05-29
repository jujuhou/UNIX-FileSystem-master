#include "head.h"
#include "error.h"
//初始化整个文件系统空间

bool Init()
{
	//在当前目新建文件作为文件卷
	fstream fd(DISK_NAME, ios::out );
	fd.close();
	fd.open(DISK_NAME, ios::out|ios::in|ios::binary);

	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//对SuperBlock进行初始化
	SuperBlock superblock; 
	superblock.s_inodenum = INODE_NUM;
	//总块数 = Block数 + SuperBlock所占块数 + Inode位图所占块数+ Inode所占块数
	superblock.s_blocknum = BLOCK_NUM+ BLOCK_POSITION;
	superblock.s_inodesize = INODE_SIZE;
	superblock.s_blocksize = BLOCK_SIZE;
	superblock.s_finodenum = INODE_NUM - 2;//有两个已经被用
	superblock.s_fblocknum = BLOCK_NUM - 2;//有两个已经被用
	//直接管理的Block空间（0-49）
	for (int i = 0; i < FREE_BLOCK_GROUP_NUM; i++) 
		superblock.s_free[i] = FREE_BLOCK_GROUP_NUM - 1 - i;
	superblock.s_nfree = FREE_BLOCK_GROUP_NUM - 1 - 2;//有两个已经被用
	//写入内存
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
	
	//进行成组连接（保证500/50整除）
	unsigned int stack[FREE_BLOCK_GROUP_NUM];
	for (int i = 2; i <= BLOCK_NUM / FREE_BLOCK_GROUP_NUM;i++) {
		for (unsigned j = 0; j < FREE_BLOCK_GROUP_NUM; j++) {
			stack[j] = FREE_BLOCK_GROUP_NUM * i - 1 - j;
		}
		if(i== BLOCK_NUM / FREE_BLOCK_GROUP_NUM)
			stack[0] = 0;
		//写入内存
		if (i != BLOCK_NUM / FREE_BLOCK_GROUP_NUM)
			fd.seekg((BLOCK_POSITION+ stack[0]- FREE_BLOCK_GROUP_NUM) * BLOCK_SIZE, ios::beg);
		else
			fd.seekg((BLOCK_POSITION + BLOCK_NUM - FREE_BLOCK_GROUP_NUM-1) * BLOCK_SIZE, ios::beg);
		fd.write((char*)&stack, sizeof(stack));
	}

	//初始化位图（初始化前两个为1，剩下的在定义时已经被置为0）
	unsigned int inode_bitmap[INODE_NUM] = { 0 };
	inode_bitmap[0] = 1;
	inode_bitmap[1] = 1;
	//写入内存
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&inode_bitmap, sizeof(inode_bitmap));

	//创建根目录（目录的权限无值）
	Inode Inode_root;
	Inode_root.i_number=0;//Inode的编号
	Inode_root.i_addr[0]=0;//对应0号Block
	Inode_root.i_mode=Inode::IDIRECTORY;//目录
	Inode_root.i_count=0;//引用计数
	Inode_root.i_uid=0;//管理员
	Inode_root.i_gid=1;//文件所有者的组标识
	Inode_root.i_size=0;//目录大小为0
	Inode_root.time= time(NULL);//最后访问时间
	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&Inode_root, sizeof(Inode_root));

	//创建用户文件
	Inode Inode_accounting;
	Inode_accounting.i_number = 1;//Inode的编号
	Inode_accounting.i_addr[0] = 1;//对应1号Block
	Inode_root.i_mode = Inode::IFILE;//文件
	Inode_root.i_count = 0;//引用计数
	Inode_root.i_permission = 0700;//管理员可读写
	Inode_root.i_uid = 0;//管理员
	Inode_root.i_gid = 1;//文件所有者的组标识
	Inode_root.i_size = 0;//目录大小为0
	Inode_root.time = time(NULL);//最后访问时间
	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE+INODE_SIZE, ios::beg);
	fd.write((char*)&Inode_accounting, sizeof(Inode_accounting));

	//创建两个账户
	User user;
	strcpy(user.u_name[0], "root");
	strcpy(user.u_password[0], "root");
	strcpy(user.u_name[1], "juju");
	strcpy(user.u_password[1], "juju");
	for (int i = 0; i < USER_NUM; i++) {//0 代表管理员
		user.u_id[i] = i;
	}
	user.u_gid[0] = 1;
	user.u_gid[1] = 2;
	//写入内存
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE + Inode_accounting.i_addr[0]*BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));

	return true;
	fd.close();
}