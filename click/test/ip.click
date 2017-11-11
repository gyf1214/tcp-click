require(library ../network/net2.click)

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
    RatedSource(DATA "hello1", RATE 1) -> IPEncap(6, 192.168.17.1, 192.168.17.3, TTL 64) -> output
}

sender2 :: {
    RatedSource(DATA "hello2", RATE 1) -> IPEncap(6, 192.168.17.3, 192.168.17.1, TTL 64) -> output
}

front1, front2, front3 :: InfraFrontend()
back1 , back2 , back3  :: InfraBackend()
router1 :: RouterCore(IP 192.168.17.1)
router2 :: RouterCore(IP 192.168.17.2)
router3 :: RouterCore(IP 192.168.17.3)

veth1 => front1 -> router1 -> back1 => veth1
veth4 => front3 -> router3 -> back3 => veth4

veth2, veth3 => front2 -> router2 -> back2 => veth2, veth3

sender1 -> router1
sender2 -> router3
