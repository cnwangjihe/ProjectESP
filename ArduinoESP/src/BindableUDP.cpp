#include "Arduino.h"
#include "BindableUDP.h"

extern "C" {
#include "user_interface.h"
#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/igmp.h"
}

bool BindableUDP::connectbind(ip_addr_t *addr, uint16_t port, ip_addr_t *local_addr, uint16_t local_port)
{
    close();
    _pcb = udp_new();
    if(_pcb == NULL) {
        return false;
    }
    err_t err = udp_bind(_pcb, local_addr, local_port);
    if(err != ERR_OK) {
        close();
        return false;
    }
    err = udp_connect(_pcb, addr, port);
    if(err != ERR_OK) {
        close();
        return false;
    }
    udp_recv(_pcb, &_s_recv, (void *) this);
    _connected = true;
    return true;
}

bool BindableUDP::connectbind(const IPAddress addr, uint16_t port, const IPAddress local_addr, uint16_t local_port)
{
    ip_addr_t daddr, dladdr;
    daddr.addr  = addr;
    dladdr.addr = local_addr;
    return connectbind(&daddr, port, &dladdr, local_port);
}


bool BindableUDP::connectbind(ip_addr_t *addr, uint16_t port, uint16_t local_port)
{
    ip_addr_t dladdr;
    dladdr.addr = IPAddress(0,0,0,0);
    return connectbind(addr, port, &dladdr, local_port);
}

bool BindableUDP::connectbind(const IPAddress addr, uint16_t port, uint16_t local_port)
{
    ip_addr_t daddr;
    daddr.addr  = addr;
    return connectbind(&daddr, port, local_port);
}