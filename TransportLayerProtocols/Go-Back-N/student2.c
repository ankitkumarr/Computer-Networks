#include <stdio.h>
#include <stdlib.h>
#include "project2.h"
#include <string.h>
#define WINDOW_SIZE (8) 
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

//Array of size assuming that the number of packets will be less than 50000 and it can be changed as required
struct pkt array[50000];

int nextseq = 1;   //tracks the nextsequence to be stored
int base = 1;	//the base of the window

int chksum(struct pkt packet); //prototype for chksum

void A_output(struct msg message) {

	//create a packet with the message in it
	struct pkt packettosend;
	packettosend.seqnum = nextseq;
	packettosend.acknum = 0;
	strncpy(packettosend.payload, message.data, 20);
	packettosend.checksum = chksum(packettosend);
	//store the packet in the array
	array[nextseq] = packettosend;
	if(nextseq < (base + WINDOW_SIZE)) {
		tolayer3(AEntity, packettosend);
	
		if (base == nextseq) {
			startTimer(AEntity, 1000);
		}
	}

	nextseq++;

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

	//proceed only if non-corrupted ACK is received
	if(packet.checksum == chksum(packet)) {
		//if the packet is out of order, do not proceed, or else if is the end, stop the timer
		if(base >= (packet.seqnum + 1)) {
			if (packet.seqnum +1 == nextseq) {
				stopTimer(AEntity);
			}
		}
		else {
			base = packet.seqnum + 1;
			//End of window reached, stop the timer
			if (base == nextseq) {
				stopTimer(AEntity);
			}
			else {
				//restart the timer after a proper ACK is received and the window is still open
				stopTimer(AEntity);
				startTimer(AEntity, 1000);
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
	//After every timeout, send the entire window of packets again!
	int i = base;
	for(; i < nextseq, i < (base + WINDOW_SIZE) ; i++) {
		tolayer3(AEntity, array[i]); //will send all the packets to layer3
	}
	startTimer(AEntity, 1000);

}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
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

//global variables for B
struct pkt lastack;
int expectedseqnum = 1;

void B_input(struct pkt packet) {

	struct pkt packettosend;
	struct msg message;
	//proceed only if packet is not corrupt
	if(packet.checksum == chksum(packet)) {
		if(packet.seqnum != expectedseqnum) {		//if not the expected sequence, send the most recent expeceted packet received
			packettosend = lastack;
			tolayer3(BEntity, packettosend);
			return;
		}

		else {
			//expected packet arrived, so send the proper ack and the message
			packettosend.seqnum = packet.seqnum;
			packettosend.acknum = 1;
			packettosend.checksum = chksum(packettosend);
			lastack = packettosend;
			tolayer3(BEntity, packettosend);
			strncpy(message.data, packet.payload, 20);
			tolayer5(BEntity, message);
			expectedseqnum++;
		}
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


//checksum same as used in ABP
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

