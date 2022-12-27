enum messageType{
    COORD_MSG,
    NORMAL_MSG,
    TIMEOUT_MSG,
    PRINT_MSG
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
    CORRECT,
    LOSS,
    DELAYED,
    DUPLICATED,
    DELAYED_AND_DUPLICATED,
};

enum MessageState{
    WAITING,
    SENT
};
