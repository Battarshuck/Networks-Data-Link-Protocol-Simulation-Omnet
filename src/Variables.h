enum messageType{
    COORD_MSG,
    NORMAL_MSG,
    TIMEOUT_MSG
};

enum role{
    SENDER,
    RECEIVER
};


enum frameType{
    DATA,
    ACK,
    NACK
};

enum ErrorType{
    LOSS,
    DELAYED,
    DUPLICATED,
    DELAYED_AND_DUPLICATED,
    CORRECT
};
