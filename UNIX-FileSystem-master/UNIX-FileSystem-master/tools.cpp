#include "head.h"
#include "error.h"

//初始化整个文件系统空间
void Init()
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
	superblock.s_finodenum = INODE_NUM - 2;//有两个已经被用
	superblock.s_fblocknum = BLOCK_NUM - 2;//有两个已经被用
	//直接管理的Block空间（0-49）
	for (int i = 0; i < FREE_BLOCK_GROUP_NUM; i++) 
		superblock.s_free[i] = FREE_BLOCK_GROUP_NUM - 1 - i;
	superblock.s_nfree = FREE_BLOCK_GROUP_NUM - 1 - 2;//有两个已经被用
	//写入内存
	fd.seekg(SUPERBLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&superblock, sizeof(superblock));
	
	//进行成组链接
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
	//写入内存
	fd.seekg(INODE_BITMAP_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)inode_bitmap, sizeof(unsigned int) * INODE_NUM);

	//创建根目录
	Inode Inode_root;
	Inode_root.i_number=0;//Inode的编号
	Inode_root.i_addr[0]=0;//对应0号Block
	Inode_root.i_mode=Inode::IDIRECTORY;//目录
	Inode_root.i_count=0;//引用计数
	Inode_root.i_uid=0;//管理员
	Inode_root.i_gid=1;//文件所有者的组标识
	Inode_root.i_size=0;//目录大小为0
	Inode_root.i_time= time(NULL);//最后访问时间
	Inode_root.i_permission = 0777;
	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE, ios::beg);
	fd.write((char*)&Inode_root, sizeof(Inode_root));
	//0号Block写入Directory
	Directory root_directory;
	strcpy(root_directory.d_filename[0],".");//0是自己
	root_directory.d_inodenumber[0] = 0;
	strcpy(root_directory.d_filename[1], "..");//1是父亲（此处还是自己）
	root_directory.d_inodenumber[1] = 0;
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		root_directory.d_filename[i][0] = '\0';
	}
	for (int i = 2; i < SUBDIRECTORY_NUM; i++) {
		root_directory.d_inodenumber[i] = -1;
	}
	fd.seekg(BLOCK_POSITION*BLOCK_SIZE, ios::beg);
	fd.write((char*)&root_directory, sizeof(root_directory));

	//创建用户文件
	Inode Inode_accounting;
	Inode_accounting.i_number = 1;//Inode的编号
	Inode_accounting.i_addr[0] = 1;//对应1号Block
	Inode_accounting.i_mode = Inode::IFILE;//文件
	Inode_accounting.i_count = 0;//引用计数
	Inode_accounting.i_permission = 0700;//管理员可读写
	Inode_accounting.i_uid = 0;//管理员
	Inode_accounting.i_gid = 1;//文件所有者的组标识
	Inode_accounting.i_size = 0;//目录大小为1
	Inode_accounting.i_time = time(NULL);//最后访问时间
	//写入内存
	fd.seekg(INODE_POSITION * BLOCK_SIZE+INODE_SIZE, ios::beg);
	fd.write((char*)&Inode_accounting, sizeof(Inode_accounting));

	//创建两个账户
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
	//写入内存
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE + Inode_accounting.i_addr[0]*BLOCK_SIZE, ios::beg);
	fd.write((char*)&user, sizeof(user));
	
	//进入根目录
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&directory, sizeof(directory));
	
	fd.close();

	//以root用户创建文件目录
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

//打开文件系统
void Activate()
{
	//打开文件卷
	fd.open(DISK_NAME, ios::out | ios::in | ios::binary);
	//如果没有打开文件则输出提示信息并throw错误
	if (!fd.is_open()) {
		cout << "无法打开文件卷myDisk.img" << endl;
		throw(ERROR_CANT_OPEN_FILE);
	}

	//读取当前目录
	fd.seekg(BLOCK_POSITION * BLOCK_SIZE, ios::beg);
	fd.read((char*)&directory, sizeof(directory));

	//将所有inode的打开数清零
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
	cout << "有关某个命令的详细信息，请键入 HELP 命令名" << endl;
	cout << "HELP           帮助文档。" << endl;
	cout << "ATTRIB         显示或更改文件属性。" << endl;
	cout << "CD             显示当前目录的名称或将其更改。" << endl;
	cout << "DEL            删除至少一个文件。" << endl;
	cout << "DIR            显示一个目录中的文件和子目录。" << endl;
	cout << "EXIT           退出文件系统。" << endl;
	cout << "MKDIR          创建一个目录。" << endl;
	cout << "RMDIR          删除目录。" << endl;
	cout << "PRINT          打印文件内容。" << endl;
	cout << "WRITE          向文件中写入内容" << endl;
	cout << "OPEN           打开一个文件" << endl;
	cout << "CLOSE          关闭一个文件" << endl;
	cout << "CREATE         创建一个文件" << endl;
	cout << "OPENLIST       当前打开文件列表" << endl;
	cout << "FSEEK          更改一个文件的指针" << endl;
	cout << "LOGOUT         用户退出登录" << endl;
	cout << "WHOAMI         显示当前用户信息" << endl;
	cout << "FORMAT         格式化文件卷" << endl;
	cout << "REGISTER       用户注册" << endl;
	cout << "DELETEACCOUNT  删除用户（root用户下）" << endl;
	cout << "SU             改变用户" << endl;
	cout << "CHGRP          改变用户所属组（root用户下）" << endl;
	cout << "USERLIST       显示所有用户信息（root用户下）" << endl;
	cout << "" << endl;
	cout << "" << endl;
	cout << "" << endl;

}

void help_attrib()
{
	cout << "更改文件的读写属性" << endl;
	cout << endl;
	cout << "ATTRIB [+R | -R] [+W | -W] [+E | -E] [O | G | E] [path][filename]" << endl;
	cout << endl;
	cout << "  +   设置属性。" << endl;
	cout << "  -   清除属性。" << endl;
	cout << "  R   读文件属性。" << endl;
	cout << "  W   写文件属性。" << endl;
	cout << "  E   执行文件属性。" << endl;
	cout << "  /O  文件所有者用户。" << endl;
	cout << "  /G  文件同组者用户。" << endl;
	cout << "  /E  非文件所有者、非文件同组者用户" << endl;
	cout << "[path][filename]" << endl;
	cout << "      指定属性要处理的文件。" << endl;
}

void help_cd()
{
	cout << "显示当前目录名或改变当前目录。" << endl;
	cout << endl;
	cout << "CD [path]" << endl;
	cout << endl;
	cout << "不带参数只键入 CD，则显示当前驱动器和目录。" << endl;
	cout << "path规则：.代表本目录，..代表父目录，路径间以/或\\分隔" << endl;
}
void help_del()
{
	cout << "删除一个或多个文件。" << endl;
	cout << endl;
	cout << "DEL names" << endl;
	cout << endl;
	cout << "删除多个或一个文件。" << endl;
	cout << "有用户未关闭文件时不能删除文件。" << endl;
}
void help_dir()
{
	cout << "显示目录中的文件和子目录列表。" << endl;
	cout << endl;
	cout << "DIR [/Q]" << endl;
	cout << endl;
	cout << "  /Q  显示详细信息。" << endl;
}
void help_exit()
{
	cout << "退出程序(文件系统)。" << endl;
	cout << endl;
	cout << "DIR [exitCode]" << endl;
	cout << endl;
	cout << "  exitCode    指定一个数字号码。如果退出，则用那个数字设置过程退出代码。" << endl;
}
void help_mkdir()
{
	cout << "创建目录。" << endl;
	cout << endl;
	cout << "MKDIR dir" << endl;
	cout << endl;
	cout << "  dir    创建的目录名称。" << endl;
}
void help_rmdir()
{
	cout << "删除一个目录。" << endl;
	cout << endl;
	cout << "MKDIR dir" << endl;
	cout << endl;
	cout << "  dir    待删除的目录名称（仅有空目录可以被删除）。" << endl;
}
void help_print()
{
	cout << "打印文本文件。" << endl;
	cout << endl;
	cout << "PRINT [filename] [path]" << endl;
	cout << endl;
	cout << "  filename 打印的文件名称。" << endl;
	cout << "  path     可选，打印到文件系统所在目录的文件名称。" << endl;
}
void help_write()
{
	cout << "打印文本文件。" << endl;
	cout << endl;
	cout << "PRINT [filename] [-s|-f] [path|content]" << endl;
	cout << endl;
	cout << "  filename 写入的文件名称。" << endl;
	cout << "  -s      从屏幕写入。" << endl;
	cout << "  -f      从文件写入。" << endl;
	cout << "  path    写入到文件系统所在目录的文件名称。。" << endl;
	cout << "  content 写入文件内容。" << endl;
}
void help_open()
{
	cout << "打开文件。" << endl;
	cout << endl;
	cout << "OPEN [filename]" << endl;
	cout << endl;
	cout << "  filename 打开的文件名称。" << endl;
}
void help_close()
{
	cout << "关闭文件。" << endl;
	cout << endl;
	cout << "CLOSE [filename]" << endl;
	cout << endl;
	cout << "  filename 关闭的文件名称。" << endl;
}
void help_fseek()
{
	cout << "更改文件指针。" << endl;
	cout << endl;
	cout << "FSEEK filename pos" << endl;
	cout << endl;
	cout << "  filename 文件名称。" << endl;
	cout << "  pos      文件指针定位位置。" << endl;
}
void help_create()
{
	cout << "创建文件。" << endl;
	cout << endl;
	cout << "CREATE filename" << endl;
	cout << endl;
	cout << "  filename 文件名称。" << endl;
}
void help_logout()
{
	cout << "用户登出。" << endl;
	cout << endl;
	cout << "LOGOUT" << endl;
}
void help_whoami()
{
	cout << "获取当前登录用户。" << endl;
	cout << endl;
	cout << "WHOAMI" << endl;
}
void help_format()
{
	cout << "格式化文件卷。" << endl;
	cout << endl;
	cout << "FORMAT" << endl;
}
void help_register()
{
	cout << "创建新用户。" << endl;
	cout << endl;
	cout << "REGISTER username password" << endl;
	cout << "  username 用户名。" << endl;
	cout << "  password 用户密码。" << endl;
}
void help_deleteaccount()
{
	cout << "删除用户。" << endl;
	cout << endl;
	cout << "DELETEACCOUNT username" << endl;
	cout << "  username 用户名。" << endl;
}
void help_su()
{
	cout << "改变用户" << endl;
	cout << endl;
	cout << "SU username password" << endl;
	cout << "  username 用户名。" << endl;
	cout << "  password 用户密码。" << endl;
}
void help_chgrp()
{
	cout << "改变用户所属组" << endl;
	cout << endl;
	cout << "CHGRP username usergroupid" << endl;
	cout << "  username    用户名。" << endl;
	cout << "  usergroupid 用户所属组。" << endl;
}
void help_userlist()
{
	cout << "显示所有用户信息" << endl;
	cout << endl;
	cout << "USERLIST" << endl;
}
void help_openlist()
{
	cout << "显示所有打开文件列表" << endl;
	cout << endl;
	cout << "OPENLIST" << endl;
}