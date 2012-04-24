//
//  EmiMessage.h
//  roshambo
//
//  Created by Per Eckerdal on 2012-04-09.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef emilir_EmiMessage_h
#define emilir_EmiMessage_h

// A message, as it is represented in the sender side of the pipeline
template<class SockDelegate>
class EmiMessage {
private:
    // Private copy constructor and assignment operator
    inline EmiMessage(const EmiMessage& other);
    inline EmiMessage& operator=(const EmiMessage& other);
    
    typedef typename SockDelegate::Binding   Binding;
    typedef typename Binding::PersistentData PersistentData;
    
    size_t _refCount;
    
    inline void commonInit() {
        _refCount = 1;
        registrationTime = 0;
        channelQualifier = EMI_CHANNEL_QUALIFIER_DEFAULT;
        sequenceNumber = 0;
        flags = 0;
        priority = EMI_PRIORITY_DEFAULT;
    }
    
    // The caller is responsible for releasing the returned object
    static EmiMessage *makeSynAndOrRstMessage(EmiFlags flags, EmiSequenceNumber sequenceNumber) {
        EmiMessage *msg = new EmiMessage;
        msg->priority = EMI_PRIORITY_HIGH;
        msg->channelQualifier = -1; // Special SYN/RST message channel. SenderBuffer requires this to be an integer
        msg->sequenceNumber = sequenceNumber;
        msg->flags = flags;
        return msg;
    }
    
public:
    
    // The caller is responsible for releasing the returned object
    static EmiMessage *makeSynMessage(EmiSequenceNumber sequenceNumber) {
        return makeSynAndOrRstMessage(EMI_SYN_FLAG, sequenceNumber);
    }
    
    // The caller is responsible for releasing the returned object
    static EmiMessage *makeSynRstMessage(EmiSequenceNumber sequenceNumber) {
        return makeSynAndOrRstMessage(EMI_SYN_FLAG | EMI_RST_FLAG, sequenceNumber);
    }
    
    // The caller is responsible for releasing the returned object
    static EmiMessage *makeRstMessage(EmiSequenceNumber sequenceNumber) {
        return makeSynAndOrRstMessage(EMI_RST_FLAG, sequenceNumber);
    }
    
    // The caller is responsible for releasing the returned object
    static EmiMessage *makeDataMessage(EmiChannelQualifier cq,
                                       EmiSequenceNumber sequenceNumber,
                                       const PersistentData& data,
                                       EmiPriority priority) {
        EmiMessage<SockDelegate> *msg = new EmiMessage<SockDelegate>(data);
        msg->channelQualifier = cq;
        msg->sequenceNumber = sequenceNumber;
        msg->priority = priority;
        
        return msg;
    }
    
    explicit EmiMessage(PersistentData data_) : data(data_) {
        commonInit();
    }
    
    EmiMessage() : data() {
        commonInit();
    }
    
    ~EmiMessage() {
        Binding::releasePersistentData(data);
    }
    
    inline void retain() {
        _refCount++;
    }
    
    inline void release() {
        _refCount--;
        if (0 == _refCount) delete this;
    }
    
    size_t approximateSize() const {
        // + 2 for the possibility of adding ACK data to the message
        // + 3 for the sequence number and split id
        return EMI_HEADER_LENGTH + Binding::extractLength(data) + 2 + 3;
    }
    
    // THIS FIELD IS INTENDED TO BE USED ONLY BY EmiSenderBuffer!
    // Modifying this field outside of that class will break invariants
    // and can result in behaviour ranging from mild inefficiencies and
    // stalled message streams to hard crashes.
    EmiTimeInterval registrationTime;
    // This is int32_t and not EmiChannelQualifier because it has to be capable of
    // holding -1, the special SYN/RST message channel as used by EmiSenderBuffer
    int32_t channelQualifier;
    EmiSequenceNumber sequenceNumber;
    EmiFlags flags;
    EmiPriority priority;
    const PersistentData data;
};

#endif
