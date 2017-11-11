require(library ip03.click)

sender2 :: {
    RatedSource(DATA "hello3-->1", RATE 1) -> IPEncap(6, 192.168.17.3, 192.168.17.1, TTL 64) -> output
}

sender3 :: {
    RatedSource(DATA "hello2-->3", RATE 1) -> IPEncap(6, 192.168.17.2, 192.168.17.3, TTL 64) -> output
}

sender4 :: {
    RatedSource(DATA "hello3-->2", RATE 1) -> IPEncap(6, 192.168.17.3, 192.168.17.2, TTL 64) -> output
}

sender5 :: {
    RatedSource(DATA "hello4-->5", RATE 1) -> IPEncap(6, 192.168.17.4, 192.168.17.5, TTL 64) -> output
}

sender6 :: {
    RatedSource(DATA "hello6-->2", RATE 1) -> IPEncap(6, 192.168.17.6, 192.168.17.2, TTL 64) -> output
}

sender1 -> router1
sender2 -> router3
sender3 -> router2
sender4 -> router3
sender5 -> router4
sender6 -> router6
