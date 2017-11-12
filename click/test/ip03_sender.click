require(library ip03.click)

sender2 :: {
    RatedSource(DATA "hello3-->1", RATE 1) -> IpSender(6, 192.168.17.1) -> output
}

sender3 :: {
    RatedSource(DATA "hello2-->3", RATE 1) -> IpSender(6, 192.168.17.3) -> output
}

sender4 :: {
    RatedSource(DATA "hello3-->2", RATE 1) -> IpSender(6, 192.168.17.2) -> output
}

sender5 :: {
    RatedSource(DATA "hello4-->5", RATE 1) -> IpSender(6, 192.168.17.5) -> output
}

sender6 :: {
    RatedSource(DATA "hello6-->2", RATE 1) -> IpSender(6, 192.168.17.2) -> output
}

sender1 -> [1] router1
sender2 -> [1] router3
sender3 -> [1] router2
sender4 -> [1] router3
sender5 -> [1] router4
sender6 -> [1] router6
Idle()  -> [1] router5
