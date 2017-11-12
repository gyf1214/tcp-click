sock :: SimpleSocket(IP 192.168.17.1)

Idle() -> sock -> Print(self, -1, PRINTANNO true) -> Discard()
sock [1] -> Socket(TCP, 0.0.0.0, 8888) -> Queue() -> [1] sock
