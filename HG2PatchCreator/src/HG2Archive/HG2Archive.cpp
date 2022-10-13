/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#include "HG2Archive.h"

#include <sys/stat.h>
#include <stdlib.h> 
#include <search.h>
#include <stdio.h>

static std::uint8_t header[0xC] = { 0x48, 0x47, 0x32, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00 };
static std::uint8_t padding[0x10] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static std::uint8_t fill[0x4] = { 0xAA, 0xAA, 0xAA, 0xAA };
static std::uint8_t padding1[0x8] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

template <class Compare>
void sort(Compare comp);
bool comp(const cHG2Archive::Entry& a, const cHG2Archive::Entry& b)
{
	return (a.m_sid < b.m_sid);
}

cHG2Archive::cHG2Archive()
{
	m_pFileWriter = nullptr;
	m_numEntries = 0;
	m_pFirstFile = 0;
	m_nameTableOffset = 0;
}

cHG2Archive::~cHG2Archive()
{
	if (m_pFileWriter)
	{
		m_pFileWriter->Close();
		delete m_pFileWriter;
	}
	m_entries.clear();
}

int cHG2Archive::Init()
{
	int ret = 1;
	m_pFileWriter = new Tools::FileWriter("patch.bin");
	if (m_pFileWriter == nullptr)
	{
		ret = -1;
	}
	return ret;
}

void cHG2Archive::FillEntries(int count, const char* aArgv[])
{
	Entry entry;
	int tot = count -1;
	for (int i = 1; i < tot; i += 2)
	{
		printf("Processing %s (%s)..\n", aArgv[i], aArgv[i + 1]);
		entry.m_fileSize = GetSize(aArgv[i]);		//get the fileSize
		entry.m_sid = StringIdHash(aArgv[i + 1]);	//get the filepath sid
		entry.m_relativeOffset = 0xFEEDFEED;
		entry.m_pFileName = aArgv[i];
		entry.m_pFilePath = aArgv[i + 1];
		m_entries.push_back(entry);
	}
	m_entries.sort(comp);
}

void cHG2Archive::CreatePatch()
{
	std::ostream* pStream = m_pFileWriter->GetStream();
	//write header
	pStream->write(reinterpret_cast<const char*>(header), 0xC);
	m_pFirstFile = (long)pStream->tellp();
	//here we'll store the pointer to the first file of the archive.
	pStream->write(reinterpret_cast<const char*>(fill), 0x4);
	//write padding
	pStream->write(reinterpret_cast<const char*>(padding), 0x10);
	//file count stored @ 0x20
	pStream->write(reinterpret_cast<const char*>(fill), 0x4);
	//write padding
	pStream->write(reinterpret_cast<const char*>(padding1), 0x8);
	//name pointer stored table @ 0x2C
	pStream->write(reinterpret_cast<const char*>(fill), 0x4);
	//write entries
	WriteEntries();
	m_numEntries = m_entries.size();
	//grab the starting offset of the name table
	m_nameTableOffset = (long)pStream->tellp();
	//allocate space for the name ptrs..
	AllocateSpaceForNameTablePtrs();

	WriteNames();

	WriteNameTablePtrs();
	//go to the end of the file
	pStream->seekp(0,std::ios_base::end);
	//eventually align
	m_pFileWriter->Align(0x4);

	WriteFiles();

	PatchOffsets();
}

void cHG2Archive::PatchOffsets(void)
{
	std::ostream* pStream = m_pFileWriter->GetStream();
	//number of files in the archive
	pStream->seekp(0x20, SEEK_SET);
	pStream->write(reinterpret_cast<const char*>(&m_numEntries), 0x4);
	//store the ptr to the name table
	pStream->seekp(0x2C, SEEK_SET);
	pStream->write(reinterpret_cast<const char*>(&m_nameTableOffset), 0x4);
	//replace the entries file offset with the real ones
	pStream->seekp(0x30, SEEK_SET);
	//update the file offset in each entry
	WriteEntries();
	//update pFirst file
	pStream->seekp(m_pFirstFile);
	pStream->write(reinterpret_cast<const char*>(&m_entries.begin()->m_relativeOffset), 0x4);
}

long cHG2Archive::GetSize(const char* fileName)
{
	FILE* fh;
	fopen_s(&fh, fileName, "rb");
	if (!fh) 
	{
		printf("Could not open %s! - Does it even exist?\n", fileName);
		return NULL;
	}
	fseek(fh, 0, SEEK_END);
	long fsize = ftell(fh);
	fclose(fh);
	return fsize;
}

void cHG2Archive::WriteEntries()
{
	std::ostream* pStream = m_pFileWriter->GetStream();
	for (std::list<Entry>::const_iterator itr = m_entries.begin();
		itr != m_entries.end();
		itr++)
	{
		pStream->write(reinterpret_cast<const char*>(&itr->m_sid), 0x4);
		pStream->write(reinterpret_cast<const char*>(&itr->m_fileSize), 0x4);
		pStream->write(reinterpret_cast<const char*>(&itr->m_relativeOffset), 0x4);
	}
}

void cHG2Archive::AllocateSpaceForNameTablePtrs()
{
	std::uint32_t pad = 0;
	std::ostream* pStream = m_pFileWriter->GetStream();
	//allocate space for the name ptrs..
	for (int i = 0; i < m_numEntries; i++)
	{
		pStream->write(reinterpret_cast<const char*>(&pad), 0x4);
	}
}

void cHG2Archive::WriteNames()
{
	std::ostream* pStream = m_pFileWriter->GetStream();
	//write the fileNames
	for (std::list<Entry>::iterator itr = m_entries.begin();
		itr != m_entries.end();
		itr++)
	{
		//store the offset where we write the name
		itr->m_nameOffset = (long)pStream->tellp();
		//write the name
		pStream->write(reinterpret_cast<const char*>(itr->m_pFilePath), strlen(itr->m_pFilePath) + 1);
	}
}

void cHG2Archive::WriteNameTablePtrs()
{
	std::ostream* pStream = m_pFileWriter->GetStream();
	pStream->seekp(m_nameTableOffset);
	for (std::list<Entry>::const_iterator itr = m_entries.begin();
		itr != m_entries.end();
		itr++)
	{
		//write the name offset
		pStream->write(reinterpret_cast<const char*>(&itr->m_nameOffset), 0x4);
	}
}

void cHG2Archive::WriteFiles()
{
	FILE* fh;
	for (std::list<Entry>::iterator itr = m_entries.begin();
		itr != m_entries.end();
		itr++)
	{
		fopen_s(&fh, itr->m_pFileName, "rb");
		if (fh)
		{
			std::int32_t fileSize = itr->m_fileSize;
			std::ostream* pStream = m_pFileWriter->GetStream();
			char* pFile = reinterpret_cast<char*>(malloc(fileSize));
			if (pFile)
			{	
				//grab the file pos in the archive
				itr->m_relativeOffset = (long)pStream->tellp();
				//read the file
				fread_s(pFile, itr->m_fileSize, 0x1, fileSize, fh);
				//write the file to the archive
				pStream->write(pFile, fileSize);
				//align
				m_pFileWriter->Align(0x4);
				//cleanup
				free(pFile);
			}
			else
			{
				printf("!!! Failed to allocate %08X bytes for file. Nothing will be written.\nPlease delete the patch.bin file since its invalid.\n", itr->m_fileSize);
			}
			fclose(fh);
		}
		else
		{
			printf("Could not open %s! - Does it even exist?\n", itr->m_pFileName);
		}
	}
}
