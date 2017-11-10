require(library common.click)

veth1 :: LossyRouterPort(DEV veth1, IN_MAC 0a:18:b4:f5:25:9d, OUT_MAC 12:25:70:5c:0f:13,
                         LOSS 0.9, DELAY 0.01)
veth2 :: LossyRouterPort(DEV veth2, IN_MAC 12:25:70:5c:0f:13, OUT_MAC 0a:18:b4:f5:25:9d,
                         LOSS 0.9, DELAY 0.01)
