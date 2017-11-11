require(library common.click)

veth1 :: RouterPort(DEV veth1, IN_MAC f2:45:7a:a0:ee:92, OUT_MAC 86:3f:83:47:d8:17)
veth2 :: RouterPort(DEV veth2, IN_MAC 86:3f:83:47:d8:17, OUT_MAC f2:45:7a:a0:ee:92)
veth3 :: RouterPort(DEV veth3, IN_MAC ca:56:53:b4:09:5c, OUT_MAC 4e:2e:c0:b9:81:a3)
veth4 :: RouterPort(DEV veth4, IN_MAC 4e:2e:c0:b9:81:a3, OUT_MAC ca:56:53:b4:09:5c)
