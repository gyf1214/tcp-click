require(library common.click)

veth1  :: LossyRouterPort(DEV veth1 , IN_MAC aa:bb:cc:dd:ee:01, OUT_MAC aa:bb:cc:dd:ee:02, LOSS 1.0, DELAY 0.2)
veth2  :: LossyRouterPort(DEV veth2 , IN_MAC aa:bb:cc:dd:ee:02, OUT_MAC aa:bb:cc:dd:ee:01, LOSS 1.0, DELAY 0.2)

veth3  :: LossyRouterPort(DEV veth3 , IN_MAC aa:bb:cc:dd:ee:03, OUT_MAC aa:bb:cc:dd:ee:04, LOSS 1.0, DELAY 0.2)
veth4  :: LossyRouterPort(DEV veth4 , IN_MAC aa:bb:cc:dd:ee:04, OUT_MAC aa:bb:cc:dd:ee:03, LOSS 1.0, DELAY 0.2)

veth5  :: LossyRouterPort(DEV veth5 , IN_MAC aa:bb:cc:dd:ee:05, OUT_MAC aa:bb:cc:dd:ee:06, LOSS 1.0, DELAY 0.2)
veth6  :: LossyRouterPort(DEV veth6 , IN_MAC aa:bb:cc:dd:ee:06, OUT_MAC aa:bb:cc:dd:ee:05, LOSS 1.0, DELAY 0.2)

veth7  :: LossyRouterPort(DEV veth7 , IN_MAC aa:bb:cc:dd:ee:07, OUT_MAC aa:bb:cc:dd:ee:08, LOSS 1.0, DELAY 0.2)
veth8  :: LossyRouterPort(DEV veth8 , IN_MAC aa:bb:cc:dd:ee:08, OUT_MAC aa:bb:cc:dd:ee:07, LOSS 1.0, DELAY 0.2)

veth9  :: LossyRouterPort(DEV veth9 , IN_MAC aa:bb:cc:dd:ee:09, OUT_MAC aa:bb:cc:dd:ee:10, LOSS 1.0, DELAY 0.2)
veth10 :: LossyRouterPort(DEV veth10, IN_MAC aa:bb:cc:dd:ee:10, OUT_MAC aa:bb:cc:dd:ee:09, LOSS 1.0, DELAY 0.2)
