require(library tcp03.click)

elementclass SClient { PORT $port |
    sock :: SimpleSocket(IP 192.168.17.1)

    input -> Print(sockout, -1, PRINTANNO true) -> sock
          -> Print(sockint, -1, PRINTANNO true) -> output
    sock [1] -> Socket(TCP, 0.0.0.0, $port) -> [1] sock
}

socket1 :: SClient(PORT 8881)
socket2 :: SClient(PORT 8882)
// socket3 :: SClient(PORT 8883)

socket1 -> [1] router1 [1] -> socket1
socket2 -> [1] router2 [1] -> socket2
Idle()  -> [1] router3 [1] -> Idle()
Idle()  -> [1] router4 [1] -> Idle()
Idle()  -> [1] router5 [1] -> Idle()
Idle()  -> [1] router6 [1] -> Idle()
