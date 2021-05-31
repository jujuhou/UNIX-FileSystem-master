#define MAIN
#include "head.h"
# include <vector>
int main()
{
	try {
		user_id = 0;
		Init();
		Activate();
		Create_File("new_file1");
		Create_File("new_file2");
		File* tmp = Open_File("new_file1");
		Write_File(tmp, "abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc");
		Show_File_List();
		Seek_File(tmp, 100);
		Write_File(tmp, "ccccccccccccccccccccccccccccccccccc");
		Show_File_List();
		char tmp_content[10000];
		int len=Read_File(tmp, tmp_content);
		cout << tmp_content<<endl;
		cout << len<<endl;

		Create_Directory("new_dir");
		Show_File_List();
		cout << endl;
		Open_Directory("./new_dir");
		Create_File("new_file3");
		Create_File("new_file4");
		Show_File_List();
		cout << endl;

		cout << "------" << endl;
		Create_Directory("new_dir2");
		Open_Directory("./new_dir2");
		Create_File("new_file5");
		Create_File("new_file6");
		Show_File_List();

		Open_Directory("..");
		Show_File_List();
	}
	catch (int& r) {
		cout << r;
	}
	

	return 0;
}