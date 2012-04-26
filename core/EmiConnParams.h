//
//  EmiConnParams.h
//  roshambo
//
//  Created by Per Eckerdal on 2012-04-23.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef roshambo_EmiConnParams_h
#define roshambo_EmiConnParams_h

#include "EmiP2PData.h"

// The main purpose of this class is to be able to
// change the number of parameters to EmiConn's
// constructor without requiring to change all bindings.
//
// With this class, one can simply add a new instance
// variable to it instead.
template<class Address>
class EmiConnParams {
public:
    EmiConnParams(const Address& address_, uint16_t inboundPort_, EmiConnectionType type_) :
    address(address_),
    inboundPort(inboundPort_),
    type(type_),
    p2p() {}
    
    EmiConnParams(const Address& address_, uint16_t inboundPort_,
                  const uint8_t *p2pCookie_, size_t p2pCookieLength_,
                  const uint8_t *sharedSecret_, size_t sharedSecretLength_) :
    address(address_),
    inboundPort(inboundPort_),
    type(p2pCookie_ && sharedSecret_ ? EMI_CONNECTION_TYPE_P2P : EMI_CONNECTION_TYPE_CLIENT),
    p2p(p2pCookie_, p2pCookieLength_, sharedSecret_, sharedSecretLength_) {}
    
    const Address& address;
    const uint16_t inboundPort;
    const EmiConnectionType type;
    EmiP2PData p2p;
};

#endif
