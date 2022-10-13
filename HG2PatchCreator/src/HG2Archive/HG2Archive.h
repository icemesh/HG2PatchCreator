/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#ifndef HG2ARCHIVE_H_
#define HG2ARCHIVE_H_

#include "../filewriter/filewriter.h"
#include "../stringid/stringid.h"

class cHG2Archive
{
public:
	cHG2Archive();

	~cHG2Archive();
	/*
	* Creates the file writer
	*	
	* @return			1 for normal termination. Returns -1 on error.
	*/
	int Init();

	/*
	* Fill a list with the files to be added to the archive
	*
	* @param count		number of arguments passed to the main
	* @param aArgv[]	arguments passed to the main
	*/
	void FillEntries(int count, const char* aArgv[]);

	/*
	* Creates a patch.bin file
	*/
	void CreatePatch();

	/*
	* Patches different offsets in the patch.bin
	*/
	void PatchOffsets(void);

	/*
	* Gets the filesize of a given filename
	*
	* @return			the file size of the given file.  Returns NULL on error.
	*/
	long GetSize(const char* fileName);

	/*
	* Writes the entries in the header of the file
	*/
	void WriteEntries();

	/*
	* Allocate space for the name table where we'll store the ptrs to the various filenames
	*/
	void AllocateSpaceForNameTablePtrs();

	/*
	* Writes the entries fileNames
	*/
	void WriteNames();

	/*
	* Writes the ptr to the various filenames to the previously allocated name table
	*/
	void WriteNameTablePtrs();

	/*
	* Writes the files to the archive
	*/
	void WriteFiles();

	struct Entry
	{
		StringId		m_sid;
		std::uint32_t	m_relativeOffset;
		std::int32_t	m_fileSize;
		const char*		m_pFileName;
		const char*		m_pFilePath;
		long			m_nameOffset;
	};
	Tools::FileWriter*						m_pFileWriter;
	std::list<Entry>						m_entries;
	int										m_numEntries;
	long									m_pFirstFile;
	long									m_nameTableOffset;
};

#endif
