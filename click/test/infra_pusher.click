// test

pusher :: InfraPusher

pusher -> Print(recv) -> Discard

RatedSource(DATA "hello1", RATE 1) -> Print(send1) -> [0] pusher
RatedSource(DATA "hello2", RATE 1) -> Print(send2) -> [1] pusher
