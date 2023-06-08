#ifndef CRC16_H
#define CRC16_H

#include <boost/crc.hpp>

size_t crc16(void* data, std::size_t length)
{
    boost::crc_16_type result;
    result.process_bytes(data, length);
    return result.checksum();
}

#endif