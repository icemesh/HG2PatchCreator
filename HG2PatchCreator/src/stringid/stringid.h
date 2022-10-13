/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/

#ifndef STRINGID_H_
#define STRINGID_H_

#include <cinttypes>

#define kInvalidStringId 0xFFFFFFFF

extern unsigned int g_crc32Table[];

inline const std::uint32_t StringIdHashConcat(std::uint32_t base, const char* str)
{
	do{
		base = (base >> 0x8) ^ g_crc32Table[(std::uint8_t)base ^ *str++];
	} while (*str);
	return base;
}

inline const std::uint32_t StringIdHash(const char* str)
{
	return (str && *str) ? StringIdHashConcat(0xFFFFFFFF, str) : kInvalidStringId;
}

typedef std::uint32_t StringId;

#define SID(str) (StringIdHash(str))


#endif
