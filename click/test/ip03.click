require(library ../network/net3.click)

elementclass RouterCore { IP $ip |
    ipfront  :: IpFrontend(IP $ip)
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)
    lspback  :: LspBackend(IP $ip)

    input -> ipfront -> output
    ipfront [1] -> lspfront -> output
    lspfront [1] -> lspback -> [1] ipfront
    ipfront [2] -> Print(self, -1) -> Discard()
}

sender1 :: {
    RatedSource(DATA "hello1-->3", RATE 1) -> IPEncap(6, 192.168.17.1, 192.168.17.3, TTL 64) -> output
}

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

front1, front2, front3, front4, front5, front6  :: InfraFrontend()
back1 , back2 , back3 , back4 , back5 , back6   :: InfraBackend()
router1 :: RouterCore(IP 192.168.17.1)
router2 :: RouterCore(IP 192.168.17.2)
router3 :: RouterCore(IP 192.168.17.3)
router4 :: RouterCore(IP 192.168.17.4)
router5 :: RouterCore(IP 192.168.17.5)
router6 :: RouterCore(IP 192.168.17.6)

veth1 => front1 -> router1 -> back1 => veth1
veth6 => front2 -> router2 -> back2 => veth6
veth10 => front3 -> router3 -> back3 => veth10

veth4, veth5 => front4 -> router4 -> back4 => veth4, veth5
veth8, veth9 => front5 -> router5 -> back5 => veth8, veth9

veth2, veth3, veth7 => front6 -> router6 -> back6 => veth2, veth3 ,veth7

sender1 -> router1
sender2 -> router3
sender3 -> router2
sender4 -> router3
sender5 -> router4
sender6 -> router6

Script(TYPE ACTIVE,
    wait 1,
    // shutdown router4
    write front4.power 0,
    write back4.power 0,
    print "shutdown router4"
    //
)
