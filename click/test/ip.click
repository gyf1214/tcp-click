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

sender :: {
    RatedSource(DATA "hello", RATE 1) -> IPEncap(6, 192.168.17.1, 192.168.17.3, TTL 64) -> output
}

veth1, sender => InfraFrontend() -> RouterCore(IP 192.168.17.1) -> InfraBackend() => veth1
veth2, veth3 => InfraFrontend() -> RouterCore(IP 192.168.17.2) -> InfraBackend() => veth2, veth3
veth4 => InfraFrontend() -> RouterCore(IP 192.168.17.3) -> InfraBackend() => veth4
