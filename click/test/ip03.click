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
