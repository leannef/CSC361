/* CSC361 ASSIGNMENT2 P2A
	Leanne Feng
	V00825004
*/
Design Questions:
1. I created a packet_field struct where contains all the important information to share:
#define ACK 0
#define DAT 1
#define FIN 2
#define RST 3
#define SYN 4
struct packet_field {
	char *magic;
   	int type;
    	int seqno;
	int ackno;
	int length;
	int windowSize;
	char *empty_line;
};

struct packet_field packet;

For magic and empty_line are just strings so I use char* to output them. I use integer for type So I can use packet.type to switch state such as Synchronizing, waiting for ACK, waiting for DAT and so on. (I define each state as an integer ex,if packet.type == SYN ...). Then for the left of content I used integer as well because they can be represented by numbers. And I set their max buffer length to 1024. Because it will be enough space for packet data. Since sequence number, ackno , length will be much shorter length than windowSize so 1024 is also enough for them. 

2. I use packet.type to store which current state is receiver or sender is. For example, If(packet.type == ACk) in sender, it means sender recerives ACK segment from receiver and can now send the packet data to receiver. I will randomly choose the initial sequence number.

3. Only receiver uses window size so receiver will adjust the window size. If the sysytem is overloaded receiver will reuse or shrink the window size. Then sender will enforce stop sending too many data. If receiver sees everything back to normal condition then it will increase window size and annouce sender that it is ok to send data. I use fopen to open the file, fread and fseek to read the file and fwrite to write, fclose to close the file. Receiver can adjust buffer size, sender can justify buffer size based on this parameter as well.

4. By using retransmition mechansim can hanle the error detection, notification and recovery. I use multiple timers. I start a timer when sending out a packet, on acknowledgement "covering" this packet cancel the timer and setup another one. If timer timeouts it will mean packet may be lost. event1: data received from application: create segment with sequence number NextSeqNum if the timer is not running, strat timer. Else pass segment to IP, NextSeqNum = NextSeqNum + length(data). event2:timer timeout: Retransmit not-ye-acknowledged segment with smallest sequence number and start timer. event3: ACk received, with ACK field vlaue of y. If y < SendBase (The state variable SendBase is the Seq# of the oldest unacknowledged byte) SendBase = y if there are currently any not-yet-acknowledged segments, start timer. 
Use acknowledge number to ensure reliable data transfer.

5. How can I get infromation in receiver when I send all the sequence number, type, and etc in one buffer?
