//
//  EmiConnectionOpenedBlockWrapper.m
//  eminet
//
//  Created by Per Eckerdal on 2012-06-13.
//  Copyright (c) 2012 Per Eckerdal. All rights reserved.
//

#import "EmiConnectionOpenedBlockWrapper.h"

@implementation EmiConnectionOpenedBlockWrapper {
    __weak id<EmiConnectionDelegate> _delegate;
    dispatch_queue_t _delegateQueue;
    NSData *_cookie;
    NSData *_sharedSecret;
    id _userData;
}

@synthesize delegate = _delegate;
@synthesize delegateQueue = _delegateQueue;
@synthesize cookie = _cookie;
@synthesize sharedSecret = _sharedSecret;
@synthesize userData = _userData;

- (id)initWithDelegate:(__weak id<EmiConnectionDelegate>)delegate
         delegateQueue:(dispatch_queue_t)delegateQueue
                cookie:(NSData *)cookie
          sharedSecret:(NSData *)sharedSecret
              userData:(id)userData {
    if (self = [super init]) {
        _delegate = delegate;
        _delegateQueue = delegateQueue;
        _cookie = cookie;
        _sharedSecret = sharedSecret;
        _userData = userData;
    }
    
    return self;
}

+ (EmiConnectionOpenedBlockWrapper *)wrapperWithDelegate:(__weak id<EmiConnectionDelegate>)delegate
                                           delegateQueue:(dispatch_queue_t)delegateQueue
                                                  cookie:(NSData *)cookie
                                            sharedSecret:(NSData *)sharedSecret
                                                userData:(id)userData {
    return [[EmiConnectionOpenedBlockWrapper alloc] initWithDelegate:delegate
                                                       delegateQueue:delegateQueue
                                                              cookie:cookie
                                                        sharedSecret:sharedSecret
                                                            userData:userData];
}

@end
