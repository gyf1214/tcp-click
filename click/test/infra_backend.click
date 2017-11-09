backend :: InfraBackend

RatedSource(DATA "hello1", RATE 1)
    -> SetAnnoByte(2, 0) -> SetAnnoByte(3, 0)
    -> [0] backend

RatedSource(DATA "hello2", RATE 1)
    -> SetAnnoByte(2, 2) -> SetAnnoByte(3, 0)
    -> [1] backend

backend [0] -> Print(recv1) -> Discard
backend [1] -> Print(recv2) -> Discard
backend [2] -> Print(recv3) -> Discard
