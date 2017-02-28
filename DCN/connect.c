/*
 * Bluetooth Programming for conecting to application running on the mobile
*/


/* ************************* Include files *************************** */
#include "bluetooth_header.h"

int main()
{
	char addr[19] = { 0 };
	char name[248] = { 0 };
	
	
	char *uuid_str= "66841278-c3d1-11df-ab31-001de000a901";
	//char *uuid_str= "00011010-0000-1000-8000-00805F9B34FB";
	int i, sock, dev_id = -1;
	//int err;
	struct hci_dev_info dev_info;
	inquiry_info *info = NULL;
	int num_rsp, how_long, flags;
	uuid_t uuid = { 0 };
	int loco_channel = -1 , receive_break = 0;
		
	uint32_t Magic_number = 0x0000ffff;
	sdp_list_t *response_list = NULL, *search_list, *attrid_list;
		

sdc:	num_rsp = 0;
		loco_channel = -1;
		memset(addr,'\0',sizeof(addr)); memset(name,'\0',sizeof(name));
		//strcpy(addr,"\0");	strcpy(name,"\0"); strcpy(dev_info,"\0");
		i= 0; sock = 0, dev_id = -1; info = NULL;
	dev_id = hci_get_route(NULL);
	if (dev_id < 0)
	{
		perror("No Bluetooth Adapter Available");
		exit(1);
	}

	if (hci_devinfo(dev_id, &dev_info) < 0)
	{
		perror("Can't get device info");
		exit(1);
	}

	sock = hci_open_dev( dev_id );
	if (sock < 0)
	{
		perror("HCI device open failed");
		free(info);
		exit(1);
	}

	
	if( !str2uuid( uuid_str, &uuid ) )
	{
		perror("Invalid UUID");
		free(info);
		exit(1);
	}

	do {
		fprintf(stdout,"Scanning For devices...\n");
		info = NULL;
		num_rsp = 0;
		flags = 0;
		how_long = 2;
		num_rsp = hci_inquiry(dev_id, how_long, num_rsp, NULL, &info, flags);
		if (num_rsp < 0)
		{
			perror("Inquiry failed");
			exit(1);
		}

		fprintf(stdout,"No of resp %d\n",num_rsp);
		if(num_rsp > 0)
		{
		for (i = 0; i < num_rsp; i++)
		{
			sdp_session_t *session;
			int retries;
			int foundit, responses;
			ba2str(&(info+i)->bdaddr, addr);
			memset(name, 0, sizeof(name));
			if (hci_read_remote_name(sock, &(info+i)->bdaddr, sizeof(name), name, 0) < 0)
			strcpy(name, "[unknown]");
			printf("Found %s  %s, searching for application now\n", addr, name);

//--------------- connecting to the SDP server running on mobile-----------
//sdpconnect:
			session = 0; retries = 0;
			while(!session)
			{
				session = sdp_connect( BDADDR_ANY, &(info+i)->bdaddr, SDP_RETRY_IF_BUSY );
				if(session) break;
				if(errno == EALREADY && retries < 5)
				{
					perror("Retrying");
					retries++;
					sleep(1);
					continue;
				}
				break;
			}
			if ( session == NULL )
			{
				perror("Can't open session with the device");
				free(info);
				continue;
			}
			search_list = sdp_list_append( 0, &uuid );
			attrid_list = sdp_list_append( 0, &Magic_number );
			//err = 0;
			//err = sdp_service_search_attr_req( session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
			sdp_service_search_attr_req( session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
			sdp_list_t *r = response_list;
			sdp_record_t *rec;
			// go through each of the service records
			foundit = 0;
			responses = 0;
			for (; r; r = r->next )
			{
					responses++;
					rec = (sdp_record_t*) r->data;
					sdp_list_t *proto_list;
					
					// get a list of the protocol sequences
					if( sdp_get_access_protos( rec, &proto_list ) == 0 ) 
					{
					sdp_list_t *p = proto_list;

						// go through each protocol sequence
						for( ; p ; p = p->next )
						{
								sdp_list_t *pds = (sdp_list_t*)p->data;

								// go through each protocol list of the protocol sequence
								for( ; pds ; pds = pds->next )
								{

										// check the protocol attributes
										sdp_data_t *d = (sdp_data_t*)pds->data;
										int proto = 0;
										for( ; d; d = d->next )
										{
												switch( d->dtd )
												{ 
														case SDP_UUID16:
														case SDP_UUID32:
														case SDP_UUID128:
																proto = sdp_uuid_to_proto( &d->val.uuid );
																break;
														case SDP_UINT8:
																if( proto == RFCOMM_UUID )
																{
																	fprintf(stdout,"rfcomm channel: %d\n",d->val.int8);
																	loco_channel = d->val.int8;
																	//printf("\nthe value of loco_channel is: %d\n",loco_channel);
																	foundit = 1;
																}
																break;
												}
										}
								}
								sdp_list_free( (sdp_list_t*)p->data, 0 );
						}
						sdp_list_free( proto_list, 0 );

					}
					if (loco_channel > 0)
						break;

			}
			static int w = 0;
			w++; 
			fprintf(stdout,"%d ",w);	
			fprintf(stdout,"No of Responses %d\n", responses);
			if ( loco_channel > 0 && foundit == 1 )
			{
				foundit = 0;
				fprintf(stdout,"Found Apllication on this device\n");
				
				s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
				fprintf(stdout,"\n");
				while(1)
				{
					send_fun( i,loco_channel,info);
					receive_break = receive_fun();
					if(receive_break < 0)
					break;
				}
				
				sdp_record_free( rec );
				//break;
			}close(s);
			
			sdp_close(session);
			
			//break;
		}
			
		sdp_list_free(response_list,0);
		sdp_list_free(search_list,0);
		sdp_list_free(attrid_list,0);
		}
		if(receive_break < 0)
		break;
		if (loco_channel > 0)
			{
				goto sdc;
			}
	} while (1);

	fprintf(stdout,"Application closed. Exiting form the application...\n");
}
