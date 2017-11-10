require(library ../network/net2.click)

elementclass RouterCore { IP $IP |
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)
    lspback  :: LspBackend(IP $ip)

    input -> lspfront -> output
    lspfront [1] -> lspback -> Print(routing) -> TimedSink(1)
}

elementclass Router1 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend

    input -> frontend
    backend -> output

    frontend -> RouterCore(IP $ip) -> backend
}

elementclass Router2 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend

    input [0] -> [0] frontend
    input [1] -> [1] frontend
    backend [0] -> [0]output
    backend [1] -> [1]output

    frontend -> RouterCore(IP $ip) -> backend
}

veth1 => Router1(IP 192.168.17.1) => veth1
veth2, veth3 => Router2(IP 192.168.17.2) => veth2, veth3
veth4 => Router1(IP 192.168.17.3) => veth4
