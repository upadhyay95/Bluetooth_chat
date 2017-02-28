/* ************************* Include files *************************** */
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/*

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/mgmt.h>    
*/


#include "bluetooth/bluetooth.h"
#include "bluetooth/hci.h"
#include "bluetooth/hci_lib.h"
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include "bluetooth/rfcomm.h"
#include "bluetooth/mgmt.h"  


#define FILEPATH_NAME "./Send-file.txt"

int s = 0;

int str2uuid( const char *, uuid_t * );
int receive_fun();
void send_fun( int i, int, inquiry_info *);

/* ******* Finding UUID of application runing on mobile ******** */

int str2uuid( const char *uuid_str, uuid_t *uuid ) 
{
    uint32_t uuid_int[4];
    char *endptr;

    if( strlen( uuid_str ) == 36 )
    {
//------ Parsing my uuid128 standard format: 12345678-9012-3456-7890-123456789012
        char buf[9] = { 0 };

        if( uuid_str[8] != '-' && uuid_str[13] != '-' &&  uuid_str[18] != '-'  && uuid_str[23] != '-' )
        return 0;
// -------------------------- first 8-bytes --------------------------------
        strncpy(buf, uuid_str, 8);
        uuid_int[0] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

// -------------------------- second 8-bytes --------------------------------
		memset(buf,'\0',sizeof(buf));
        strncpy(buf, uuid_str+9, 4);
        strncpy(buf+4, uuid_str+14, 4);
        uuid_int[1] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

// -------------------------- third 8-bytes --------------------------------
        memset(buf,'\0',sizeof(buf));
        strncpy(buf, uuid_str+19, 4);
        strncpy(buf+4, uuid_str+24, 4);
        uuid_int[2] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

// -------------------------- fourth 8-bytes --------------------------------
        memset(buf,'\0',sizeof(buf));
        strncpy(buf, uuid_str+28, 8);
        uuid_int[3] = htonl( strtoul( buf, &endptr, 16 ) );
        if( endptr != buf + 8 ) return 0;

        if( uuid != NULL )
        sdp_uuid128_create( uuid, uuid_int );
    }

    return -1;
}

int receive_fun()
{

	char buf[1024] = { 0 };
	int status = 0;
	
	//fprintf(stdout," \t-----------------Inside receive function-------\n");						
	
	//sleep(5);
	status = read(s, buf, sizeof(buf));
	//fprintf(stdout,"Received %d bytes\n", status);	
	//fprintf(stdout,"%s\n", buf);	
	fprintf(stdout,"Received: %s\n", buf);	
	//sleep(2);
	return (status);
}

void send_fun( int i,int ptr,inquiry_info *info)
{
	struct sockaddr_rc loc_addr = { 0 };
	int loco_channel = -1, status = 0;
	char buff[1024] = { 0 };
	static char one_time_socket_creation = 0;
	FILE* filedes = NULL;
	
	loco_channel = ptr;
	//fprintf(stdout,"Printing loco_channel value %d\n",loco_channel);
	
	
	loc_addr.rc_family = AF_BLUETOOTH;
	loc_addr.rc_channel = loco_channel;
	loc_addr.rc_bdaddr = *(&(info+i)->bdaddr);

	ba2str(&loc_addr.rc_bdaddr, buff );
	//fprintf(stdout, "\nSending Data to %s\n", buff);
	
	//sleep(60);
	
	one_time_socket_creation++;
	if(one_time_socket_creation == 1)
	status = connect(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
		
	//printf("Value of connect is: %d\n",status);
	if( status < 0 )
	{
	perror("Some error ocured");
	exit(1);
	}  
	//fprintf(stdout,"\n%d\n",status);
	do {

	memset(buff,'\0',sizeof(buff));
	fprintf(stdout,"Me:       ");
	fgets(buff,sizeof(buff),stdin);
	status = send(s,buff,sizeof(buff),MSG_CONFIRM);
	static int cnt = 0;
	cnt = cnt+1;
	status = cnt;
	//fprintf(stdout,"%d\n",cnt);
	sleep(1); // my testing purpose
	//}while (status > 0);
	}while (status < 1);
	//close(s);	
	
	//return (&s);
}
