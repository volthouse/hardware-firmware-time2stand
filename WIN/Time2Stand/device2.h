#ifndef DEVICE2_H
#define DEVICE2_H

#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#include <windows.h>

struct Data {
    uint16_t distance;
    uint16_t time;
};

class Device2 {
public:

    static Device2& Instance() {
        static Device2 instance;
        return instance;
    }

    std::error_code Open(const std::string &portName, uint32_t baudrate);

    std::error_code ResetData();
    std::error_code ReceiveData(Data &data);

    std::error_code Send(const std::string& send);
    std::error_code Send(void *buffer, const uint32_t len);
    std::error_code Receive(void *buffer, const uint32_t len);

    void Purge();

    void Close();

private:
    Device2();
    ~Device2();

    int configure();

    std::string portNameToSystemLocation(const std::string& source);
    bool calculateChecksum(const std::vector<uint8_t>& buffer);
    std::wstring stringToWString(const std::string& str);

    std::string m_portname;
    uint32_t m_baudrate;
    uint32_t m_read_timeout_ms = 2000;
    uint32_t m_send_timeout_ms = 1000;
    HANDLE m_handle;
};

#endif // DEVICE2_H
