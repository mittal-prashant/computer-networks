# Sliding Window Protocol

### Important Notes:

* Selective Repeat method for Sliding Window Protocol has been implemented in the assignment.
* Please ensure there is server.py and client.py file before running the code.
* Run the server.py file first using command: `python3 server.py`
* Run the client.py file second using command: `python3 client.py`
* Follow the steps that are displayed during execution.
* To end the transmission of packets give "stop" as input.

### Insights of Code:

* Window size has been set to 10.
* Due to a little ambiguity in question I've assumed that the packet is lost by server end with probabilty 0.3.
* I've implemented the fact that packet is lost with probabity 0.3.
* For a large number of packets being sent the Server starts losing/dropping packets by 30%.
* If the packet is dropped no acknowedgment is sent and the timeout occurs which tells client that the packet has been dropped and we need to resend the packet again.
* Client: Press enter to send the packets or stop to stop transmission of packets.
* Server: Shows logs for packets received/dropped.
