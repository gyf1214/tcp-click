frontend :: InfraFrontend

frontend -> Print(recv, PRINTANNO true) -> Discard

RatedSource(DATA "hello1", RATE 1) -> Print(send1) -> [0] frontend
RatedSource(DATA "hello2", RATE 1) -> Print(send2) -> [1] frontend
