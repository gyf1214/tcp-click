require(library tcp03.click)

elementclass SSimple {IP $ip, PORT $port |
    sock :: SimpleSocket(IP $ip)

    input -> sock -> output
    sock [1] -> Socket(TCP, 0.0.0.0, $port) -> [1] sock
}

elementclass SServer { IP $ip, PORT $port |
    sock :: SocketServer(IP $ip, PORT $port, INTERVAL 0.2, WAIT 0, BUFFER 4096)
    input -> sock -> output
}

elementclass SClient { IP $ip, PORT $port, DST $dst, DPORT $dport |
    sock :: SocketSender(IP $ip, SPORT $port,
        DST $dst, DPORT $dport, DATA hello,
        INTERVAL 0.01, WAIT 5, LIMIT 100000, BUFFER 4096)
    input -> sock -> output
}

socket1 :: SServer(IP 192.168.17.1, PORT 5678)
socket2 :: SSimple(IP 192.168.17.2, PORT 8882)
socket3 :: SClient(IP 192.168.17.3, PORT 8899, DST 192.168.17.1, DPORT 5678)
socket4 :: SSimple(IP 192.168.17.4, PORT 8883)

socket1 -> [1] router1 [1] -> socket1
socket2 -> [1] router2 [1] -> socket2
socket3 -> [1] router3 [1] -> socket3
socket4 -> [1] router4 [1] -> socket4
Idle()  -> [1] router5 [1] -> Idle()
Idle()  -> [1] router6 [1] -> Idle()
