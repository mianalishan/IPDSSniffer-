#pragma once

#include<iostream>
#include <string.h>
#include <pcap.h>
#include <winsock2.h>


/* 4 bytes IP address */
typedef struct ip_address {
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header {
	u_char  ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
	u_char  tos;            // Type of service 
	u_short tlen;           // Total length 
	u_short identification; // Identification
	u_short flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
	u_char  ttl;            // Time to live
	u_char  proto;          // Protocol
	u_short crc;            // Header checksum
	ip_address  saddr;      // Source address
	ip_address  daddr;      // Destination address
	u_int   op_pad;         // Option + Padding
}ip_header;

/* UDP header*/
typedef struct udp_header {
	u_short sport;          // Source port
	u_short dport;          // Destination port
	u_short len;            // Datagram length
	u_short crc;            // Checksum
}udp_header;

//#define HAVE_REMOTE
//"Simple" struct for TCP
typedef struct tcp_header {
	u_short sport; // Source port
	u_short dport; // Destination port
	u_int seqnum; // Sequence Number
	u_int acknum; // Acknowledgement number
	u_char th_off; // Header length
#define TH_OFF(th)      (((th)->th_off & 0xf0) >> 4)
	u_char flags; // packet flags
	u_short win; // Window size
	u_short crc; // Header Checksum
	u_short urgptr; // Urgent pointer...still don't know what this is...

}tcp_header;
#define LINE_LEN 16

class live_sniffing
{
public:

	pcap_if_t *alldevs;
	pcap_if_t *d;
	char errbuf[PCAP_ERRBUF_SIZE];
	int i = 0;
	pcap_if_t get_Network_interfaces()
	{
		if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
		{
			fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
			exit(1);
		}

		/* Print the list */
		for (d = alldevs; d; d = d->next)
		{
			//printf("%d. %s \n", ++i, d->name);

			if (d->description)
				printf("%d (%s)     \n", ++i, d->description, d->name);
			else
				printf(" (No description available)\n");

		}
	}
	int sniffing()
	{
		pcap_if_t *alldevs;
		pcap_if_t *d;
		int inum;
		int i = 0;
		pcap_t *adhandle;
		char errbuf[PCAP_ERRBUF_SIZE];

		/* Retrieve the device list on the local machine */
		if (pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &alldevs, errbuf) == -1)
		{
			fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
			exit(1);
		}

		/* Print the list */
		for (d = alldevs; d; d = d->next)
		{
			//printf("%d. %s \n", ++i, d->name);

			if (d->description)
				printf("%d (%s)     \n", ++i, d->description, d->name);
			else
				printf(" (No description available)\n");

		}

		if (i == 0)
		{
			printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return -1;
		}

		printf("Enter the interface number (1-%d):", i);
		scanf_s("%d", &inum);

		if (inum < 1 || inum > i)
		{
			printf("\nInterface number out of range.\n");
			/* Free the device list */
			pcap_freealldevs(alldevs);
			return -1;
		}

		/* Jump to the selected adapter */
		for (d = alldevs, i = 0; i< inum - 1; d = d->next, i++);

		/* Open the device */
		if ((adhandle = pcap_open(d->name,          // name of the device
			65536,            // portion of the packet to capture
							  // 65536 guarantees that the whole packet will be captured on all the link layers
			PCAP_OPENFLAG_PROMISCUOUS,    // promiscuous mode
			1000,             // read timeout
			NULL,             // authentication on the remote machine
			errbuf            // error buffer
		)) == NULL)
		{
			fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
			/* Free the device list */
			pcap_freealldevs(alldevs);
			return -1;
		}

		printf("\nlistening on %s...\n", d->description);

		/* At this point, we don't need any more the device list. Free it */
		pcap_freealldevs(alldevs);

		/* start the capture */
		pcap_loop(adhandle, 0, dispatcher_handler, NULL);

		return 0;
	}
	//*********************

	void static dispatcher_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
	{
		struct tm ltime;
		char timestr[16];
		ip_header *ih;
		udp_header *uh;
		u_int ip_len;
		u_short sport, dport;
		time_t local_tv_sec;
		tcp_header *th;

		/*
		* Unused variable
		*/
		(VOID)(param);

		/* convert the timestamp to readable format */
		local_tv_sec = header->ts.tv_sec;
		localtime_s(&ltime, &local_tv_sec);
		strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);
		//cout << "total number of packet :  " << live_sniffing::total_packet++ << endl;
		/* print timestamp and length of the packet */
		//printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);

		/* retireve the position of the ip header */
		ih = (ip_header *)(pkt_data); //length of ethernet header

									  /* retireve the position of the udp header */
		ip_len = (ih->ver_ihl & 0xf) * 4;

		//cout<<"   *  IP header length "<<ip_len<<"  BYTES "<<endl;
		//uh = (udp_header *) ((u_char*)ih + ip_len);
		//cout<<"UDP header length "<<uh->len<<endl;

		th = (struct tcp_header *)((u_char *)ih + ip_len);
		u_int size_tcp = TH_OFF(th) * 4;

		if (size_tcp < 20) {
			printf("   * Invalid TCP header length: %u bytes\n", size_tcp);


		}
		else {
			//printf("   * valid TCP header length: %u bytes\n", size_tcp);
			//printf("   * SEQ NUMBER : %u \n",ntohl( th->seqnum));

			//cout<<"   * seq num "<<dec<<ntohl(th->seqnum)<<endl;


		}
		/* convert from network byte order to host byte order */
		sport = ntohs(th->sport);
		dport = ntohs(th->dport);
		/* print ip addresses and udp ports */



		//cout<<"tcp flag "<<hex<<th->flags<<endl;
		//cout << "   *  IP header length " << ip_len << "  BYTES " << endl;
		printf("   * valid TCP header length: %u bytes\n", size_tcp);
		//cout << "   * ip address of packet ";
		printf("%d.%d.%d.%d.  -> %d.%d.%d.%d. \n",
			ih->saddr.byte1,
			ih->saddr.byte2,
			ih->saddr.byte3,
			ih->saddr.byte4,
			ih->daddr.byte1,
			ih->daddr.byte2,
			ih->daddr.byte3,
			ih->daddr.byte4);
		//cout << "    * source port : " << dec << ntohs(th->sport) << "   des port : " << dec << ntohs(th->dport) << endl;
		//printin a data 
		if ((sport == 5001) || (dport == 5001)) {
			//cout << "ipds data in tcp packet " << endl;
			
			int payload = ip_len + size_tcp;
			FILE *of;
			fopen_s(&of, "E:\\pacPayload2.txt", "ab");
			//fwrite();
			//fprintf(of,"%.d \n",total_packet);
			//cout << "ok";
			//string temp(reinterpret_cast<const char*>(pkt_data));
			//cout << "ok";
			//string data=temp.substr(54, header->caplen);
			//cout << "ok";
			//const char* temp2 = data.c_str();
			//cout << "ok";
			printf("%.d", header->caplen - payload);

			//memcpy(&temp, &pkt_data, sizeof(pkt_data[0])+1);
			//fwrite(pkt_data ,sizeof(pkt_data[0]) , header->caplen-payload  , of);
			//ipds_message im;

			//#pragma omp parallel for 
			for (int i = payload + 1; (i <= header->caplen); i++)
			{


				//printf("%.2x ", pkt_data[i - 1]);
				fwrite(&pkt_data[i - 1], sizeof(pkt_data[0]), 1, of);
				//fprintf(of,"%.2x",pkt_data[i-1]);


				//if ((i % LINE_LEN) == 0) { printf("\n"); }

			}

			if (payload != header->caplen)
			{

				//fprintf(of,"*\n");

			}
			//fprintf(of,"\n          *******************************************        \n");
			fclose(of);


			//cout << endl << "    ************************************************                  " << endl;


		}
		else
		{
			//cout << " no concern  " << endl;
		}
	}
};
