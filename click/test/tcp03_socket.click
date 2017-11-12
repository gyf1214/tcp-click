require(library tcp03.click)

socket1 :: {
    sock :: SimpleSocket(IP 192.168.17.1)

    input -> sock -> Print(sockout, -1, PRINTANNO true) -> output
    sock [1] -> Socket(TCP, 0.0.0.0, 8888) -> [1] sock
}

socket1 -> [1] router1 [1] -> socket1
Idle()  -> [1] router2 [1] -> Idle()
Idle()  -> [1] router3 [1] -> Idle()
Idle()  -> [1] router4 [1] -> Idle()
Idle()  -> [1] router5 [1] -> Idle()
Idle()  -> [1] router6 [1] -> Idle()
