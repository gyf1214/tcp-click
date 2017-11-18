//
// Compound class that provide a port abstraction for a router
//

elementclass BDRouterPort { DEV $dev, IN_MAC $in_mac, OUT_MAC $out_mac, DELAY $delay, BAND $band, QUEUE $queue |

//Add filtering based on ips and macs
out_dev :: ToDevice($dev)
in_dev :: FromDevice($dev)

//in and out queues
in_queue::Queue();
// very small queue
out_queue_data::Queue($queue);

input -> EtherEncap(0x0800, $in_mac, $out_mac)
      -> SetTimestamp()
      -> out_queue_data
      -> LinkUnqueue($delay, $band)
      -> Queue()
      -> out_dev

//missing the arp part
in_dev -> in_queue
       -> Unqueue
       -> HostEtherFilter($in_mac, DROP_OWN true) // check that the mac address is proper
       -> Strip(14)
       -> output

}
