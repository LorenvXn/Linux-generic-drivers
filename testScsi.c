#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <scsi/sg.h> 


#define TIMEOUT 20000
#define INQ_REPLY_LEN 0xff
#define INQ_CMD_CODE 0x12
#define INQ_CMD_LEN 6
#define MAX 32


int main(int argc, char **argv)
{
	int sg_fd, k, evpd, page_code;
	unsigned char InquiryCMD[INQ_CMD_LEN] = {INQ_CMD_CODE, evpd & 1 ,  page_code & 0xff , 0, INQ_REPLY_LEN , 0};
	unsigned char InquiryBuffer[INQ_REPLY_LEN];
	unsigned char sense_buffer[MAX];

	sg_io_hdr_t point_io_hdr;

	if ((sg_fd = open(argv[1], O_RDONLY)) < 0) {
    	perror("error opening given file name");
    	return 1;
	}

	/* Prepare INQUIRY command */

	memset(&point_io_hdr, 0, sizeof(sg_io_hdr_t));

	point_io_hdr.interface_id = 'S';
	point_io_hdr.cmd_len = sizeof(InqueryCMD);
	point_io_hdr.mx_sb_len = sizeof(sense_buffer);
	point_io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;
	point_io_hdr.dxfer_len = INQ_REPLY_LEN;
	point_io_hdr.dxferp = InquiryBuffer;
	point_io_hdr.cmdp = InquiryCMD;
	point_io_hdr.sbp = sense_buffer;
	point_io_hdr.timeout = TIMEOUT;	 

	if (ioctl(sg_fd, SG_IO, &point_io_hdr) < 0) {
    	perror("sg_simple0: Inquiry SG_IO ioctl error");
    	return 1;
	}

	if ((point_io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
    	if (point_io_hdr.sb_len_wr > 0) {
        	printf("INQUIRY sense data: ");
        	for (k = 0; k < point_io_hdr.sb_len_wr; ++k) {
            	if ((k > 0) && (0 == (k % 10)))
                	printf("\n  ");
            	printf("0x%02x ", sense_buffer[k]);
        	}
        	printf("\n");
    	}
    	if (point_io_hdr.masked_status)
        	printf("INQUIRY SCSI status=0x%x\n",point_io_hdr.status);
    	if (point_io_hdr.host_status)
        	printf("INQUIRY host_status=0x%x\n", point_io_hdr.host_status);
    	if (point_io_hdr.driver_status)
        	printf("INQUIRY driver_status=0x%x\n", point_io_hdr.driver_status);
	}
	else {  
    	char * p = (char *)InqueryBuffer;
    	printf("INQUIRY command's response:\n");
    	printf("	%.8s  %.16s  %.4s\n", p + 8, p + 16, p + 32);
    	printf("INQUIRY duration=%u millisecs, resid=%d\n",
           	point_io_hdr.duration, point_io_hdr.resid);
	}
	close(sg_fd);
	return 0;
}
