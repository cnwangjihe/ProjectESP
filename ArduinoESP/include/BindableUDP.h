#include "ESPAsyncUDP.h"

class BindableUDP : public AsyncUDP{
public:
    bool connectbind(ip_addr_t *addr, uint16_t port, ip_addr_t *local_addr, uint16_t local_port);
    bool connectbind(const IPAddress addr, uint16_t port, const IPAddress local_addr, uint16_t local_port);
    bool connectbind(ip_addr_t *addr, uint16_t port, uint16_t local_port);
    bool connectbind(const IPAddress addr, uint16_t port, uint16_t local_port);
};