#define MAIN
#include "head.h"
# include <vector>
int main()
{
	try {
		user_id = 0;
		Init();
		Activate();
		Create_File("new_file");
		Create_File("new_file2");
		Show_File_List();
		user_id = 1;
		Create_File("new_file3");
		Create_File("new_file4");
		Create_File("new_file5");
		Create_File("new_file6");
		Create_File("new_file7");
		Create_File("new_file8");
		Create_File("new_file9");
		Show_File_List();
		Create_File("new_file10");
		Create_File("new_file11");
		Create_File("new_file12");


	}
	catch (int& r) {
		cout << r;
	}
	

	return 0;
}