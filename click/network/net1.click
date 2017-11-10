require(library common.click)

veth1 :: RouterPort(DEV veth1, IN_MAC 0a:18:b4:f5:25:9d, OUT_MAC 12:25:70:5c:0f:13)
veth2 :: RouterPort(DEV veth2, IN_MAC 12:25:70:5c:0f:13, OUT_MAC 0a:18:b4:f5:25:9d)
