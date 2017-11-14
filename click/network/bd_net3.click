require(library common.click)

veth1  :: BDRouterPort(DEV veth1 , IN_MAC aa:bb:cc:dd:ee:01, OUT_MAC aa:bb:cc:dd:ee:02,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)
veth2  :: BDRouterPort(DEV veth2 , IN_MAC aa:bb:cc:dd:ee:02, OUT_MAC aa:bb:cc:dd:ee:01,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)

veth3  :: BDRouterPort(DEV veth3 , IN_MAC aa:bb:cc:dd:ee:03, OUT_MAC aa:bb:cc:dd:ee:04,
                       DELAY 0.2 , BAND 1kbps, QUEUE 5)
veth4  :: BDRouterPort(DEV veth4 , IN_MAC aa:bb:cc:dd:ee:04, OUT_MAC aa:bb:cc:dd:ee:03,
                       DELAY 0.2 , BAND 1kbps, QUEUE 5)

veth5  :: BDRouterPort(DEV veth5 , IN_MAC aa:bb:cc:dd:ee:05, OUT_MAC aa:bb:cc:dd:ee:06,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)
veth6  :: BDRouterPort(DEV veth6 , IN_MAC aa:bb:cc:dd:ee:06, OUT_MAC aa:bb:cc:dd:ee:05,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)

veth7  :: BDRouterPort(DEV veth7 , IN_MAC aa:bb:cc:dd:ee:07, OUT_MAC aa:bb:cc:dd:ee:08,
                       DELAY 0.2 , BAND 1kbps, QUEUE 5)
veth8  :: BDRouterPort(DEV veth8 , IN_MAC aa:bb:cc:dd:ee:08, OUT_MAC aa:bb:cc:dd:ee:07,
                       DELAY 0.2 , BAND 1kbps, QUEUE 5)

veth9  :: BDRouterPort(DEV veth9 , IN_MAC aa:bb:cc:dd:ee:09, OUT_MAC aa:bb:cc:dd:ee:10,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)
veth10 :: BDRouterPort(DEV veth10, IN_MAC aa:bb:cc:dd:ee:10, OUT_MAC aa:bb:cc:dd:ee:09,
                       DELAY 0.2 , BAND 1Mbps, QUEUE 5)
