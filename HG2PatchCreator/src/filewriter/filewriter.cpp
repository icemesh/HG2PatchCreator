/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#include "filewriter.h"
#include <fstream>

namespace 
{
	static inline long Align(long pos, std::uint32_t Alignment)
	{
		return (pos + ((long)Alignment - 1)) & ~((long)Alignment - 1);
	}
}


namespace Tools
{
	FileWriter::FileWriter(const std::string& filename)
	{
		m_pStream = new std::ofstream(filename.c_str(), std::ios_base::out | std::ios_base::binary);
	}

	FileWriter::~FileWriter(void)
	{
		if (m_pStream)
		{
			delete m_pStream;
			m_pStream = 0;
		}
	}

	void FileWriter::Close(void)
	{
		if (m_pStream)
		{
			delete m_pStream;
			m_pStream = 0;
		}
	}

	int FileWriter::Align(std::uint32_t alignment)
	{
		long curPos = GetPos();
		long alignedPos = ::Align(curPos, alignment);
		long work = alignedPos - curPos;
		char tmp = 0;
		for (int i = 0; i < work; i++)
		{
			m_pStream->write(reinterpret_cast<const char*>(&tmp), 1);
		}
		return work;
	}

	long FileWriter::GetPos(void)
	{
		if (m_pStream)
		{
			long pos = (long)m_pStream->tellp();
			return pos != -1 ? pos : 0;
		}
		return 0;
	}
}
