// InfraPuller

puller :: InfraPuller

InfiniteSource(DATA "hello")
    -> Print(send)
    -> puller

puller [0] -> Print(recv1) -> Discard
puller [1] -> Print(recv2) -> Discard
puller [2] -> Print(recv3) -> Discard
