require(library ../network/net1.click)

elementclass Router1 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)

    input -> frontend
    backend -> output

    frontend -> lspfront -> backend
    lspfront [1] -> Print(seq) -> Discard
}

veth1 -> Router1(IP 192.168.17.1) -> veth1
veth2 -> Router1(IP 192.168.17.2) -> veth2
