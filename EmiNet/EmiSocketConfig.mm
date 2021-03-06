//
//  EmiSocketConfig.m
//  eminet
//
//  Created by Per Eckerdal on 2012-04-11.
//  Copyright (c) 2012 Per Eckerdal. All rights reserved.
//

#import "EmiSocketConfig.h"
#import "EmiSocketConfigInternal.h"

typedef EmiSocketConfigSC SC;

@implementation EmiSocketConfig {
    void *_sc;
}

- (id)init {
    if (self = [super init]) {
        _sc = new SC();
    }
    return self;
}

- (void)dealloc {
    if (_sc) {
        delete (SC *)_sc;
        _sc = nil;
    }
}

- (NSData *)serverAddress {
    const sockaddr_storage& ss(((SC *)_sc)->address);
    return [NSData dataWithBytes:&ss length:EmiNetUtil::addrSize(ss)];
}

- (void)setServerAddress:(NSData *)serverAddress {
    sockaddr_storage ss;
    memcpy(&ss, [serverAddress bytes], MIN([serverAddress length], sizeof(sockaddr_storage)));
    ((SC *)_sc)->address = ss;
}

- (EmiTimeInterval)connectionTimeout {
    return ((SC *)_sc)->connectionTimeout;
}

- (void)setConnectionTimeout:(EmiTimeInterval)connectionTimeout {
    ((SC *)_sc)->connectionTimeout = connectionTimeout;
}

- (EmiTimeInterval)initialConnectionTimeout {
    return ((SC *)_sc)->initialConnectionTimeout;
}

- (void)setInitialConnectionTimeout:(EmiTimeInterval)initialConnectionTimeout {
    ((SC *)_sc)->initialConnectionTimeout = initialConnectionTimeout;
}

- (float)heartbeatsBeforeConnectionWarning {
    return ((SC *)_sc)->heartbeatsBeforeConnectionWarning;
}

- (void)setHeartbeatsBeforeConnectionWarning:(float)heartbeatsBeforeConnectionWarning {
    ((SC *)_sc)->heartbeatsBeforeConnectionWarning = heartbeatsBeforeConnectionWarning;
}

- (NSUInteger)receiverBufferSize {
    return ((SC *)_sc)->receiverBufferSize;
}

- (void)setReceiverBufferSize:(NSUInteger)receiverBufferSize {
    ((SC *)_sc)->receiverBufferSize = receiverBufferSize;
}

- (NSUInteger)senderBufferSize {
    return ((SC *)_sc)->senderBufferSize;
}

- (void)setSenderBufferSize:(NSUInteger)senderBufferSize {
    ((SC *)_sc)->senderBufferSize = senderBufferSize;
}

- (BOOL)acceptConnections {
    return ((SC *)_sc)->acceptConnections;
}

- (void)setAcceptConnections:(BOOL)acceptConnections {
    ((SC *)_sc)->acceptConnections = acceptConnections;
}

- (uint16_t)serverPort {
    return ((SC *)_sc)->port;
}

- (void)setServerPort:(uint16_t)serverPort {
    ((SC *)_sc)->port = serverPort;
}

- (NSUInteger)MTU {
    return ((SC *)_sc)->mtu;
}

- (void)setMTU:(NSUInteger)MTU {
    ((SC *)_sc)->mtu = MTU;
}

- (float)heartbeatFrequency {
    return ((SC *)_sc)->heartbeatFrequency;
}

- (void)setHeartbeatFrequency:(float)heartbeatFrequency {
    ((SC *)_sc)->heartbeatFrequency = heartbeatFrequency;
}

- (SC *)sockConfig {
    return (SC *)_sc;
}

@end
