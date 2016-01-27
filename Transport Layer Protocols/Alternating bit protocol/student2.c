#include <stdio.h>
#include <stdlib.h>
#include "project2.h"
#include <string.h>
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for Project 2, unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/



/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */

int chksum(struct pkt packet);
struct pkt lastsentpacket;
int wait = 0; //0 is for no waiting, 1 is for waiting
struct pkt buffer[200];
int pos = 0;  //Rear end of the array
int front = 0; //front end of the array
struct pkt temp; //This will store all the temporary packets poped from the buffer
int callfrombuffer = 0;     // a flag variable. Will be 1 if the function A_output was called using the buffer packet
void A_output(struct msg message) {

	struct pkt packettosend;
	//if it was called from the buffer, the packettosend will be the one store from the buffer
	if(callfrombuffer == 1) {
		packettosend = temp;
		callfrombuffer = 0;
		front++;
	}
	else {
	packettosend.seqnum = (lastsentpacket.seqnum+1)%2;
	packettosend.acknum = 0;
	strncpy(packettosend.payload, message.data, 20);
	packettosend.checksum = chksum(packettosend);
	}
	////////////////////////////////////////////////
	//Copying the packet to the last sent packet
	if(wait == 0) {
	lastsentpacket.seqnum = packettosend.seqnum;
	lastsentpacket.acknum = packettosend.acknum;
	strncpy(lastsentpacket.payload, packettosend.payload, 20);
	lastsentpacket.checksum = packettosend.checksum;
	////////////////////////////////////////////////
	///////Sending the packet to the layer3 to be sent
	tolayer3(AEntity, packettosend);
	wait = 1;
	startTimer(AEntity, 5000);
	}

	else { 
		buffer[pos] = packettosend;
		if(pos%2==1) {			//properly sequening again to make sure the buffers get the proper order of the sequence
		buffer[pos].seqnum = (packettosend.seqnum+1)%2; }
		pos++;	
		 }
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {

}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {

	if(packet.checksum != chksum(packet)) {			//if corrupted, do nothing
		return;
	}

	else {
		if(packet.seqnum != lastsentpacket.seqnum) {
			// wait = 0;
			// tolayer3(AEntity, lastsentpacket);
		}

		else {
			printf("ACK received, Last sent successful\n\n");
			wait = 0;
			stopTimer(AEntity);
			if(pos>front) {
				struct msg message;
				temp = buffer[front];
				strncpy(message.data, buffer[front].payload, 20);
				callfrombuffer = 1;
				A_output(message);
			}
		}
	}

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {


	wait = 0; callfrombuffer = 0;	//reseting after all timeouts
	stopTimer(AEntity);
	lastsentpacket.seqnum = (lastsentpacket.seqnum+1)%2;
	struct msg message;
	strncpy(message.data, lastsentpacket.payload, 20);
	A_output(message);


}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {

	lastsentpacket.seqnum = 1;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */

int lastacked=1;
void B_input(struct pkt packet) {
	struct msg message;
	if(packet.checksum != chksum(packet)) {
		//printf("Received packet is corrupted, checksum = %d \n", packet.checksum);
		struct pkt packettosend;
		packettosend.seqnum = lastacked;
		packettosend.acknum = 1;
		packettosend.checksum = chksum(packettosend);
		tolayer3(BEntity, packettosend);
	}
	
	else if (packet.seqnum == lastacked) {
		//printf("Packet already received");
		struct pkt packettosend;
		packettosend.seqnum = lastacked;
		packettosend.acknum = 1;
		packettosend.checksum = chksum(packettosend);
		tolayer3(BEntity, packettosend);
	}
	else {
		strncpy(message.data , packet.payload, 20);
		//printf("Sending the packet to the application layer");
		struct pkt packettosend;
		packettosend.seqnum = packet.seqnum; 
		packettosend.acknum = 1;
		packettosend.checksum = chksum(packettosend);
		tolayer3(BEntity, packettosend);
		lastacked=packettosend.seqnum;
		tolayer5(BEntity, message);
	}
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
}


//function to properly calculate the checksum
int chksum(struct pkt packet) {
	int cword = 0;
	int ch;
	int i, j;

	packet.checksum = 0;
	char buffer[sizeof(packet)];
	memcpy(buffer, &packet, sizeof(packet));

	for(i = 0; i < sizeof(packet); i++) {
		ch = buffer[i] << 8;
		for(j = 0; j < 8; j++) {
			if ((ch & 0x8000) ^ (cword & 0x8000)) {
				cword = (cword <<= 1) ^ 4129;
			}
			else {
				cword <<= 1;
			}
			ch <<= 1;
		}
	}
	cword = cword + ((packet.acknum)*buffer[10]) + ((packet.seqnum)*buffer[7]);
	return cword;
}
