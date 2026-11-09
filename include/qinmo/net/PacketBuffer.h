#pragma once

#include "qinmo/tool/StringView.h"
#include <vector>
#include <string>
#include <stdint.h>



/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

class PacketBuffer
{
public:
    /// @param headerSize application-layer protocol header length
    /// @param bodySize message length
    /// @note if headerSize is 0, it is a normal buffer
    /// @note in receive buffer, headerSize **must** is 0.
    ///       or you can fill protocol header manually if you want to keep it
    explicit PacketBuffer(uint32_t headerSize, std::size_t bodySize = 1024);
    ~PacketBuffer();

public:
    /// @brief all length of protocol header
    uint32_t getHeadSize() const;
    /// @brief length of readable message body
    std::size_t getReadableSize() const;
    /// @brief length of writable message body
    std::size_t getWritableSize() const;

    /// @brief must be called once before parsing a new protocol
    void reset();


    /// @brief return protocol header value in host-endian by offset and size
    /// @param offset value Local at the offset from the start
    /// @param target write into target
    bool headGet8(uint32_t offset, int8_t& target) const;
    bool headGet16(uint32_t offset, int16_t& target) const;
    bool headGet32(uint32_t offset, int32_t& target) const;
    bool headGet64(uint32_t offset, int64_t& target) const;
    bool headGetString(uint32_t offset, uint32_t len, std::string& str) const;
    bool headGetAll(std::string& str) const;

    /// @brief set value in net-endian at the give offset
    /// @param offset value Local at the offset from the start
    bool headSet8(uint32_t offset, int8_t n);
    bool headSet16(uint32_t offset, int16_t n);
    bool headSet32(uint32_t offset, int32_t n);
    bool headSet64(uint32_t offset, int64_t n);
    bool headSetString(uint32_t offset, const std::string& str);


    /// @brief get value in host-endian without advancing the pointer
    bool peek8(int8_t& target) const;
    bool peek16(int16_t& target) const;
    bool peek32(int32_t& target) const;
    bool peek64(int64_t& target) const;
    bool peekString(std::size_t len, qinmo::StringView& sv) const;
    bool peekAll(qinmo::StringView& sv) const;

    /// @brief get value in host-endian within advancing the pointer
    bool retrieve8(int8_t& target);
    bool retrieve16(int16_t& target);
    bool retrieve32(int32_t& target);
    bool retrieve64(int64_t& target);
    bool retrieveString(std::size_t len, std::string& str);
    bool retrieveAll(std::string& str);

    /// @brief append value in net-endian at end of buffer
    void append(const char* data, std::size_t len);
    void append8(int8_t n);
    void append16(int16_t n);
    void append32(int32_t n);
    void append64(int64_t n);
    void appendString(const std::string& str);

    // /// @brief read / write data between file descriptor and buffer
    // /// @param writeLen use to return number of write
    // /// @param savedErrno system error code
    // /// @param chunkSize maximum bytes to read per system call, recommend close to size of net package
    // /// @return true if connecting, return false if peer Close socket or irrecoverable error
    // bool readFd(int fd, ssize_t& receiveLen, int& savedErrno, std::size_t chunkSize = 4096);
    // bool writeFd(int fd, ssize_t& sendLen, int& savedErrno);

    /// @brief read data from file descriptor into buffer once
    /// @param savedErrno errno code
    /// @param chunkSize size that you want to write, only close to size of package
    /// @return size of written to buffer this time
    ssize_t readFd(int fd, int& savedErrno, std::size_t chunkSize = 4096);
    /// @brief read data from file descriptor into buffer once
    /// @param savedErrno errno code
    /// @return size of written from buffer to fd this time
    /// @note if savedErrno is EAGAIN or EWOULDBLOCK, this is not real error
    ssize_t writeFd(int fd, int& savedErrno);

private:
    void retrieve(std::size_t len);
    void ensureWrite(std::size_t len);
    // bool disconnectError(int err);

    char* begin();
    const char* begin() const;

private:
    std::vector<char> buf_;
    std::size_t rIndex_;
    std::size_t wIndex_;

    const uint32_t headerSize_;
    uint32_t hsIndex_;

};

} // namespace net
} // namespace qinmo