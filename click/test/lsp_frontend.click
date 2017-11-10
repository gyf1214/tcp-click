require(../network/net1.click)

elementclass Router1 { IP $ip |
    frontend :: InfraFrontend
    backend  :: InfraBackend
    lspfront :: LspFrontend(IP $ip, INTERVAL 10, TIMEOUT 1)

    input -> frontend
    backend -> output

    frontend -> lspfront -> backend
    lspfront [1] -> Discard
}

vnet1 -> Router1(IP 192.168.17.1) -> vnet1
vnet2 -> Router1(IP 192.168.17.2) -> vnet2
