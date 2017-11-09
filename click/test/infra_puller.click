// InfraPuller

puller :: InfraPuller

InfiniteSource(DATA "hello")
    -> Print(send)
    -> puller

puller [0] -> Print(recv1) -> TimedSink(1000)
puller [1] -> Print(recv2) -> TimedSink(2000)
puller [2] -> Print(recv3) -> TimedSink(3000)
