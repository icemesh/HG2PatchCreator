/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#include <stdio.h>
#include "HG2Archive/HG2Archive.h"

int main(int argc, const char* aArgv[])
{
	if (argc < 2)
	{
		puts("This program creates a patch.bin file that can be used in Uncharted Golden Abyss");
		puts("Arguments: <filename> <filepath>...");
		return -1;
	}
	else
	{
		cHG2Archive* pAr = new cHG2Archive();
		if (pAr)
		{
			//init archive
			int ret = pAr->Init();
			if (ret != -1)
			{
				pAr->FillEntries(argc, aArgv);
				//Create the patch.bin
				pAr->CreatePatch();
			}
			else
			{
				puts("Failed to create the filewriter. Quitting...");
			}
			delete(pAr);
			return 0;
		}
		else
		{
			printf("Failed to allocate Archive.. quitting\n");
			return -1;
		}
	}
}
