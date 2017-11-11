require(library common.click)

veth1 :: RouterPort(DEV veth1, IN_MAC aa:bb:cc:dd:ee:01, OUT_MAC aa:bb:cc:dd:ee:02)
veth2 :: RouterPort(DEV veth2, IN_MAC aa:bb:cc:dd:ee:02, OUT_MAC aa:bb:cc:dd:ee:01)
veth3 :: RouterPort(DEV veth3, IN_MAC aa:bb:cc:dd:ee:03, OUT_MAC aa:bb:cc:dd:ee:04)
veth4 :: RouterPort(DEV veth4, IN_MAC aa:bb:cc:dd:ee:04, OUT_MAC aa:bb:cc:dd:ee:03)
