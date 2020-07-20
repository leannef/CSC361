/* CSC361 ASSIGNMENT2 P2B
	Leanne Feng
	V00825004
*/

The code contains rdpr.c, rdps.c and rdp_header.h.
To run, type make in terminal, or open two terminals type one of g++ rdpr.c -o rdpr or g++ rdps.c -o rdps
In server side: arguments format is ./rdpr [receiver_ip] [receiver_port] [filename]
In client side: ./rdps [sender_ip] [sender_port] [receiver_ip] [receiver_port] [sender_file_name]

The "main" function handles setting up the socket and calling funcstions in each stage of transfer file.
The "sendSYN" and "connection" function establish the connection
The "transfering" and "waitFor" function handles transfering the file and deals with window size, payload size .

True facts: 
1. the first random sequence number is created using rand(), it is randomly winthin 10; It changes depend on payload length
2. Max payload client can send is 900, max buffer server can recive is 1024, server will clear windowsize when it receives new packet.
3. checksum uses a derived hash function.

This is the header, which is a packet_field struct contains all the important information to share:
typedef struct packet_field
{
    char magic[7];
    char type[4];
    int synack_number;
    short length;
    short window;
    char data[MAX_DATA];
    unsigned long checksum;
} packet_field;

"make clean" will clear the generated binaries
