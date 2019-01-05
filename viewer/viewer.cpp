// viewer.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "partition.h"
#include "disk.h"
#include "ext2_fs.h"
#include "stdio.h"
#include <assert.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

FS_HANDLE mount(const char *part_name);

int main(int argc, char* argv[])
{
	FS_HANDLE h = mount("hda8");
	assert(h);
	common_file *files;
	int num_of_files;


	BOOL r = start_enum(h, "/dev", files, num_of_files);
	assert(r);
	cancel_enum(files);

	common_file f;

	char buf[141];
	resolve_sym_link(h, "/2", buf, 141);

	umount(h);

#ifdef _DEBUG
   _CrtDumpMemoryLeaks();
#endif

	return 0;
}
