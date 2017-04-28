/*
  WiFiUdp.cpp - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

extern "C" {
extern int rtl_printf(const char *fmt, ...);
}
#include <string.h>
#include "AZ3166WiFi.h"
#include "AZ3166WiFiUdp.h"
#include "AZ3166WiFiClient.h"
#include "SystemWiFi.h"
//#include "WiFiServer.h"

/* Constructor */
WiFiUDP::WiFiUDP() 
{
    _pUdpSocket = new UDPSocket();
    
    _localPort = 0;
    is_initialized = false;
}

/* Start WiFiUDP socket, listening at local port PORT */
uint8_t WiFiUDP::begin(uint16_t port) {

    if ( !is_initialized ) 
    {
        _pUdpSocket->set_blocking(false);  
        _pUdpSocket->set_timeout(5000);
        if(_pUdpSocket->open(WiFiInterface()) != 0)
        {
            return 0;
        }
        is_initialized = true;
    }
    
    if ( is_initialized )
    {
        _localPort = port;
        return 1;
    }
    else
    {
        return 0;
    }
}


/* Release any resources being used by this WiFiUDP instance */
void WiFiUDP::stop()
{
    if (!is_initialized)
        return;

    _pUdpSocket->close();
    is_initialized = false;
}

int WiFiUDP::beginPacket(const char *host, uint16_t port)
{
    // Look up the host first
    SocketAddress outEndPoint(host, port);
    if(WiFiInterface()->gethostbyname(host, &outEndPoint))
    {
        return 0;
    }
    IPAddress remote_addr;
    remote_addr.fromString(outEndPoint.get_ip_address());
    return beginPacket(remote_addr, port);
}

int WiFiUDP::beginPacket(IPAddress ip, uint16_t port)
{
    int ret;
    
    if ( !is_initialized ) 
    {
        if(_pUdpSocket->open(WiFiInterface()) != 0)
        {
            return 0;
        }
        
        _pUdpSocket->set_blocking(false);  
        _pUdpSocket->set_timeout(5000);

        is_initialized = true;
    }
    
    if ( is_initialized )
    {
        _address = new SocketAddress(ip.get_address(), port);
    }   
    return 1;       
}

int WiFiUDP::endPacket()
{
    return true;
}

size_t WiFiUDP::write(uint8_t byte)
{
    return write(&byte, 1);
}

size_t WiFiUDP::write(const uint8_t *buffer, size_t size)
{
    if ( !is_initialized ) 
        return 0;

    _pUdpSocket->sendto(*_address, (char*)buffer, size);
    return size;
}

int WiFiUDP::read()
{
    int n;
    char b;

    if (_address == NULL)
        return -1;

    SocketAddress recvAddress;
    recvAddress.set_ip_address(_address->get_ip_address());
    recvAddress.set_port(_address->get_port());
    n =  _pUdpSocket->recvfrom(&recvAddress,  &b, 1);
    return ( n != 1 )? (-1) : (int)(b);
}

int WiFiUDP::read(unsigned char* buffer, size_t len)
{
    if (_address == NULL)
        return -1;
    SocketAddress recvAddress;
    recvAddress.set_ip_address(_address->get_ip_address());
    recvAddress.set_port(_address->get_port());
    return _pUdpSocket->recvfrom(&recvAddress, (char*)buffer, len);
}

void WiFiUDP::flush()
{

}

IPAddress WiFiUDP::remoteIP()
{
    if(_address == NULL)
    {
        return INADDR_NONE;
    }
    IPAddress ip((uint8_t*)_address->get_ip_address());
    return ip;
}

uint16_t  WiFiUDP::remotePort()
{
    if(_address == NULL)
        return NULL;
    return _address->get_port();
}

WiFiUDP:: ~WiFiUDP()
{
    if(_pUdpSocket != NULL)
    {
        _pUdpSocket->close();
        delete _pUdpSocket;
    }

    if(_address != NULL)
    {
        delete _address;
    }
    is_initialized = false;
}