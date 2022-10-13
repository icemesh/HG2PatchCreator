/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#ifndef FILEWRITER_H_
#define FILEWRITER_H_

#include <iostream>
#include <list>

namespace Tools
{
	class FileWriter
	{
	public:
		FileWriter(const std::string& filename);

		~FileWriter(void);

		/*
		* Closes the file
		*/
		void Close(void);

		/*
		* Writes zero bytes to align the data size.
		* 
		* @param		alignment
		* 
		* @return		numbe of bytes written
		*/
		int Align(std::uint32_t alignment);

		/*
		* Gets the current position.
		* 
		* @return		The current byte position in the file. Returns 0 on error.
		*/
		long GetPos(void);

		std::ostream* GetStream() { return m_pStream; }

	private:
		std::ostream*					m_pStream;
	};
} // namespace Tools

#endif
