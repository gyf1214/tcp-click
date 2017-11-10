require(library ../network/net2.click)

elementclass Router1 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)
    lspback  :: LspBackend(IP $ip)

    input => frontend
    backend => output

    frontend -> lspfront -> backend
    lspfront [1] -> lspback -> Print(routing) -> TimedSink(1)
}

veth1 => Router1(IP 192.168.17.1) => veth1
veth2, veth3 => Router1(IP 192.168.17.2) => veth2, veth3
veth4 => Router1(IP 192.168.17.3) => veth4
