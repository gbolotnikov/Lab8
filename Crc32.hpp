#ifndef CRC32_H
#define CRC32_H

#include <boost/crc.hpp>

size_t crc32(void* data, std::size_t length)
{
    boost::crc_32_type result;
    result.process_bytes(data, length);
    return result.checksum();
}

#endif