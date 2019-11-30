# Simulation Assignment 1

## TEAM 4
- Akhilesh: `ns2.tcl`
- Luming: `ns2.tcl`

## Setup

- Ubuntu 19.04
- ns-2.35

_Case 1:_

- src1-R1 and R2-rcv1 end-2-end delay = 5 ms   
- src2-R1 and R2-rcv2 end-2-end delay = 12.5 ms   

_Case 2:_

- src1-R1 and R2-rcv1 end-2-end delay = 5 ms   
- src2-R1 and R2-rcv2 end-2-end delay = 20 ms     

_Case 3:_
- src1-R1 and R2-rcv1 end-2-end delay = 5 ms   
- src2-R1 and R2-rcv2 end-2-end delay = 27 ms

## Procedure

- Create simularoe object
- Turn on tracing
- Create topology
- Setup packet loss, link dynamics
- Create routing agents
- Create traffic sources
- Add post-processing procedures like nam, xgraph
- Start simulation

## Results

![results](images/sim.png)
### `TCP Vegas`

- Average throughput in Mbits/sec

.| **SRC1** | **SRC2** | Ratio
---- | ---- | ---- | ----
*Case 1* | 0.5104 | 0.3645 | 1.4003 
*Case 2* | 0.6016 | 0.2733 | 2.2017
*Case 3* | 0.6563 | 0.2186 | 3.0022

### `TCP Sack`

- Average throughput in Mbits/sec

.| **SRC1** | **SRC2** | Ratio
---- | ---- | ---- | ----
*Case 1* | 0.4589 | 0.4167 | 1.1012
*Case 2* | 0.4780 | 0.3975 | 1.2026
*Case 3* | 0.4955 | 0.3800 | 1.3037

- TCP Vegas gives better throughput that TCP Sack for the same RTT but as the RTT 
increases, the throughput for TCP Vegas decreases significantly whereas TCP Sack almost 
maintains the same throughput.
- This is apparent from the tables above.
- For case 1, where the RTT for the two sources were in the ratio of 1:2, TCP Vegas performs better than TCP Sack by a small amount. But as the RTT increased for SRC2, the throuput for Vegas decreased but Sack maintained the throughput. 

- TCP Vegas detects congestion at an incipient stage based on increasing RTT of the packets in the connection. It does not use the loss of segment to signal that there is congestion. It determines congestion by a decrease in sending rate as compared to the expected rate.
- It is also more stable than Sack. Sack uses packet losses to denote congestion. The sender continually increases the sending rate until there is a congestion and then the sender cuts back. The cycle continues and the system keeps on oscillating. 
- TCP Sack is difficult to implement since the receiver needs to send selective acknowledgements which are not currently implemented.


## References:
- https://en.wikipedia.org/wiki/TCP_Vegas
- https://inst.eecs.berkeley.edu/~ee122/fa05/projects/Project2/SACKRENEVEGAS.pdf





