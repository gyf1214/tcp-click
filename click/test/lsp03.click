require(library ../network/net3.click)

elementclass RouterCore { IP $ip |
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)
    lspback  :: LspBackend(IP $ip)

    input -> lspfront -> output
    lspfront [1] -> lspback -> Print(routing, -1) -> TimedSink(1)
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

elementclass Router3 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend

    input [0] -> [0] frontend
    input [1] -> [1] frontend
    input [2] -> [2] frontend

    backend [0] -> [0]output
    backend [1] -> [1]output
    backend [2] -> [2]output

    frontend -> RouterCore(IP $ip) -> backend
}

veth1 => Router1(IP 192.168.17.1) => veth1
veth6 => Router1(IP 192.168.17.2) => veth6
veth10 => Router1(IP 192.168.17.3) => veth10

veth4, veth5 => Router2(IP 192.168.17.4) => veth4, veth5
veth8, veth9 => Router2(IP 192.168.17.5) => veth8, veth9

veth2, veth3, veth7 => Router3(IP 192.168.17.6) => veth2, veth3, veth7
