require(library common.click)

veth1 :: RouterPort(DEV veth1, IN_MAC aa:bb:cc:dd:ee:01, OUT_MAC aa:bb:cc:dd:ee:02)
veth2 :: RouterPort(DEV veth2, IN_MAC aa:bb:cc:dd:ee:02, OUT_MAC aa:bb:cc:dd:ee:01)

veth3 :: RouterPort(DEV veth3, IN_MAC aa:bb:cc:dd:ee:03, OUT_MAC aa:bb:cc:dd:ee:04)
veth4 :: RouterPort(DEV veth4, IN_MAC aa:bb:cc:dd:ee:04, OUT_MAC aa:bb:cc:dd:ee:03)

veth5 :: RouterPort(DEV veth5, IN_MAC aa:bb:cc:dd:ee:05, OUT_MAC aa:bb:cc:dd:ee:06)
veth6 :: RouterPort(DEV veth6, IN_MAC aa:bb:cc:dd:ee:06, OUT_MAC aa:bb:cc:dd:ee:05)

veth7 :: RouterPort(DEV veth7, IN_MAC aa:bb:cc:dd:ee:07, OUT_MAC aa:bb:cc:dd:ee:08)
veth8 :: RouterPort(DEV veth8, IN_MAC aa:bb:cc:dd:ee:08, OUT_MAC aa:bb:cc:dd:ee:07)

veth9 :: RouterPort(DEV veth9, IN_MAC aa:bb:cc:dd:ee:09, OUT_MAC aa:bb:cc:dd:ee:10)
veth10 :: RouterPort(DEV veth10, IN_MAC aa:bb:cc:dd:ee:10, OUT_MAC aa:bb:cc:dd:ee:09)
