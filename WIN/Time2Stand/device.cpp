#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

#include <cwchar>
#include <QDebug>

#include "device.h"
#include "device_errorcode.h"

constexpr const char* RESET_CMD      = "RESET\n";
constexpr const char* GETDATA_CMD    = "GETDATA\n";
constexpr const char* TEST_SIT_CMD   = "TEST_SIT\n";
constexpr const char* TEST_STAND_CMD = "TEST_STAND\n";

Device::Device() : m_handle(INVALID_HANDLE_VALUE)
{
}

Device::~Device() {
    Close();
}

std::error_code Device::Open(const std::string &portName, uint32_t baudrate)
{
    int res;

    std::lock_guard<std::mutex> lock(m_mutex);

    m_portname = portNameToSystemLocation(portName);
    m_baudrate = baudrate;

    std::wstring portNameW = stringToWString(m_portname);

    m_handle = CreateFile(portNameW.c_str(), GENERIC_READ | GENERIC_WRITE,
                          0, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == m_handle) {
        fprintf(stderr, "\nUnable to open port %s", portName.c_str());
        return std::error_code(EIO, std::generic_category());
    }

    res = configure();
    if(0 != res) {
        fprintf(stderr, "\nConfiguration for port %s failed with %i",
                portName.c_str(), res);
        return std::error_code(EIO, std::generic_category());
    }

    purge();

    return DEVICE_ErrorCode::Success;
}

std::error_code Device::ResetData()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return send(RESET_CMD);
}

void Device::TestSit()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    send(TEST_SIT_CMD);
}

void Device::TestStand()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    send(TEST_STAND_CMD);
}

std::error_code Device::ReceiveData(DeviceData &data)
{
    std::error_code err;
    std::vector<uint8_t> buffer;

    std::lock_guard<std::mutex> lock(m_mutex);

    buffer.resize(sizeof(data) + 1);

    purge();

    err = send(GETDATA_CMD);
    if(err) {
        return err;
    }

    err = receive(buffer.data(), buffer.size());
    if(err) {
        return err;
    }

    if(!calculateChecksum(buffer)) {
        return DEVICE_ErrorCode::Checksum;
    }

    memcpy(&data, buffer.data(), sizeof(data) - 1);

    qDebug() << "distance: " << data.distance;

    return DEVICE_ErrorCode::Success;
}

std::error_code Device::send(const std::string &text)
{
    return send(static_cast<void*>(const_cast<char*>(text.c_str())),
                text.length());
}

std::error_code Device::send(void *buffer, const uint32_t len)
{
    DWORD written;

    BOOL bres = WriteFile(m_handle, buffer, len, &written, NULL);
    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr,"\nError sending buffer, err %lu: \n", err);
        return std::error_code(EIO, std::generic_category());
    }

    return DEVICE_ErrorCode::Success;
}

std::error_code Device::receive(void* buffer, const uint32_t len)
{
    BOOL bres;
    DWORD readLen = len;
    char* pBuffer = static_cast<char*>(buffer);

    while(readLen) {
        DWORD read;
        bres = ReadFile(m_handle, pBuffer, readLen, &read, NULL);
        if(FALSE == bres) {
            DWORD err = GetLastError();
            fprintf(stderr,"\nError receiving buffer, err %lu: \n", err);
            return std::error_code(EIO, std::generic_category());
        }
        if(0 == read) {
            return DEVICE_ErrorCode::Timeout;
        }
        readLen -= read;
        pBuffer += read;
    }

    return DEVICE_ErrorCode::Success;
}

void Device::purge()
{
    BOOL bres;
    DWORD flags = 0;

    flags |= PURGE_RXABORT | PURGE_RXCLEAR;
    flags |= PURGE_TXABORT | PURGE_TXCLEAR;

    bres = PurgeComm(m_handle, flags);

    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr,"\nPurge with err: %lu \n", err);
    }
}

void Device::Close() {
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

int Device::configure()
{
    bool bres;

    DCB dcb;
    ZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    bres = GetCommState(m_handle, &dcb);
    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr, "\nUnable to get comm state, err %lu", err);
        return -1;
    }

    dcb.fBinary = TRUE;
    dcb.fInX = FALSE;
    dcb.fOutX = FALSE;
    dcb.fAbortOnError = FALSE;
    dcb.fNull = FALSE;
    dcb.fErrorChar = FALSE;

    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    dcb.BaudRate = m_baudrate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    bres = SetCommState(m_handle, &dcb);
    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr, "\nUnable to set comm state, err %lu", err);
        return -1;
    }

    COMMTIMEOUTS commTimeouts;
    ZeroMemory(&commTimeouts, sizeof(commTimeouts));
    commTimeouts.ReadTotalTimeoutConstant = m_read_timeout_ms;
    commTimeouts.ReadIntervalTimeout = MAXDWORD;
    commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    commTimeouts.WriteTotalTimeoutConstant = m_send_timeout_ms;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;

    bres = SetCommTimeouts(m_handle, &commTimeouts);
    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr, "\nUnable to set comm timeouts, err %lu", err);
        return -1;
    }

    DWORD eventMask = 0;
    bres = SetCommMask(m_handle, eventMask);
    if(FALSE == bres) {
        DWORD err = GetLastError();
        fprintf(stderr, "\nUnable to set comm mask, err %lu", err);
        return -1;
    }

    return 0;
}

std::string Device::portNameToSystemLocation(const std::string& source)
{
    if (source.compare(0, 3, "COM") == 0) {
        return std::string("\\\\.\\") + source;
    } else {
        return source;
    }
}

bool Device::calculateChecksum(const std::vector<uint8_t> &buffer)
{
    uint8_t checksum = 0;
    uint8_t expectedChecksum = 0;

    if(buffer.empty()) {
        return false;
    }

    expectedChecksum = buffer[buffer.size() - 1];

    for(int i = 0; i < buffer.size() - 1; i++) {
        checksum += buffer[i];
    }

    if(checksum != expectedChecksum) {
        return false;
    }

    return true;
}

std::wstring Device::stringToWString(const std::string &str)
{
    size_t len = mbstowcs(nullptr, str.c_str(), 0);
    if (len == static_cast<size_t>(-1)) {
        return L"";
    }

    std::vector<wchar_t> buffer(len + 1);
    mbstowcs(buffer.data(), str.c_str(), len + 1);
    return std::wstring(buffer.data());
}

#endif
