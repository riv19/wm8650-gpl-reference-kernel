/*
 * ir_api.h - Ir API.
 *
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#include "ir_api.h"

#define IR_CMD_STOP_TX			0x00
#define IR_CMD_TX_PREP_CODE		0x01
#define IR_CMD_TX_LEARNED_CODE	0x02
#define IR_CMD_GET_KEY_FLAG		0x03
#define IR_CMD_LEARN_CODE		0x04
#define IR_CMD_STORE			0x07
#define IR_CMD_GET_REVISION		0x09
#define IR_CMD_MASTER_CLEAR		0x0a
#define IR_CMD_STANDBY			0x0b
#define IR_CMD_WAKEUP			0x0c
#define IR_CMD_SET_GPIO			0x0d
#define IR_CMD_READ_GPIO_CONF	0x0e
#define IR_CMD_WRITE_GPIO		0x0f
#define IR_CMD_READ_GPIO		0x10
#define IR_CMD_BACK_LIGHT_CTRL	0x11

#if 1 //zhangning add
#define IR_PACKAGE_COUNT 		5
#define IR_STORE_MAX_LOCATION 	13
#define IR_LEARN_DATA_LENGTH 	81
#define IR_LEARN_MAX_LOCATION 	100
#define IR_CMD_GET_LEARN_DATA 	0x12
#define IR_CMD_SET_LEARN_DATA 	0X13
#define IR_CMD_READ_DATA		0x14
#endif

#define CMDLEN			256	//the length of command buffer in function
#define REPEAT_TX_CNT	6	//if command trasmit fail, it will repeat tramsmit REPEAT_TX_CNT times

/* #define DEBUG_IR_API */

/**
 * msleep - sleep msecs.
 *
 * @msec - 
 *
 **/
int msleep( int msec )
{
	return usleep(msec*1000);
}

/**
 *	CalculateChecksum() -  Calculate Checksum
 *
 * @buf - the data for calculate checksum
 * @n - the long of buf
 *
 * returns: checksum value or -1 if error.
 *
 **/
unsigned char CalculateChecksum( BYTE *buf, int n )
{
    unsigned int sum = 0x00;
	int i = 0 ;
	if( !buf )
		return 0;
    for( i = 0; i < n; i++ )
        sum += *buf++;
    return sum ;
}

/**
 * read_data() -  Read data from serial port.
 *
 * @buf - The data from serial.
 * @length - want to read count.
 * @msec: time out(msecs).
 *
 * returns: the real read data count
 *
 **/
static int read_data( int fd, unsigned char * buf, unsigned int length, int msec )
{

#ifdef DEBUG_IR_API
	printf( "Enter:%s\n", __func__ );
#endif

	int ret = 0;
	struct timeval tv;
	fd_set rfds;
	if( fd < 0 ) 
		return -1;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);/* We want to check if the serial port has data */
	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec%1000)*1000;

#if 0
	printf("sce:%d\n",(int)tv.tv_sec);
	printf("usec:%d\n",(int)tv.tv_usec);
#endif
 
	ret = select(fd+1, &rfds, NULL, NULL, &tv) ;
	if( ret > 0 ) {
		if (FD_ISSET(fd,&rfds)) {
           ret = read(fd, buf, length);
           if (ret <= 0)
                printf("serial_port_read Error reading data");
#if 0
			printf("serial in:");
			int _i = 0 ;
			for( _i = 0; _i < length; _i++ )
			{
				printf(" %02x",buf[_i]);
			}
			printf("\n");
#endif
 
		}
	}
	else if( ret == 0 )
		;//printf("serial_port_read select timeout\n");
	else
		printf("Error in select on serial port errno = %d [%s]\n", errno, strerror(errno));
                
	return ret;
}

/**
 * serial_open - Open the serial port
 *
 * @port: the path of serial port
 * @mode: the rate of serial example 57600 19200 115200
 *
 * returns: the serial handle
 **/

int serial_open( char *port, int mode )
{
	int fd = -1;

    fd = open(port, O_RDWR | O_NONBLOCK);
    if (fd == -1 )
		return -1;

	struct termios t;
	int retval;
	retval = tcgetattr(fd, &t);
	if (retval != 0)
		printf("serial_port: Unable to get serial interface attributes\n");
#ifdef DEBUG_IR_API 
        printf("serial_port: \n lflag=0x%x\n  oflag=0x%x\n  cflag=0x%x\n  iflag=0x%x\n ispeed=0x%x\n ospeed=0x%x\n",
               t.c_lflag,
               t.c_oflag,
               t.c_cflag,
               t.c_iflag,
               cfgetispeed(&t)/*t.c_ispeed*/,
               cfgetospeed(&t)/*t.c_ospeed*/);
#endif
	t.c_lflag = 0;
	t.c_oflag = 0;
	t.c_cflag = CS8 | CLOCAL; // control settings  
	t.c_iflag = 0;

	switch( mode )
	{
        case 9600:
            cfsetispeed(&t, B9600);
            break;
		case 57600 :
			cfsetispeed(&t, B57600);
			break;
		case 19200 :
			cfsetispeed(&t, B19200);
			break;
		case 38400 :
			cfsetispeed(&t, B38400);
			break;
		default:
			cfsetispeed(&t, B115200); /* Default speed */
	}
	t.c_ospeed = t.c_ispeed;
	retval = tcsetattr(fd, TCSANOW, &t);
	if( retval == -1 ) 
		printf("serial_port: Unable to set serial interface attributes\n");

    return fd;
}


/**
 * serial_close - Close the serial port.
 *
 * @fd - 
 *
 **/
void serial_close( int fd )
{
	if( fd < 0 )
		return ;
	close(fd);
}

/**
 * receive_data() - read data from serial
 *
 * @buf: the data form serial, exclude chucksum
 *
 * returns: the count of read data from serial
 *
 */
static int receive_data(int fd, unsigned char * buf, int *length, int msec)
{
#ifdef DEBUG_IR_API
	printf( "Enter:%s\n", __func__ );
#endif
	unsigned char checksum=0 ;
	int i = 0;
	    
	if( (buf == NULL) || (length == NULL) ||(fd <= 0) )
		return -1;
	    
	if( read_data(fd,&buf[0], 1, msec) != 1 )
		return -1;

	if( read_data(fd,&buf[1], 1, WAIT_TIME_MS) != 1 )
		return -1;
    	    
	while( i < (buf[1]-1) )
	{
		if ( read_data(fd,&buf[i+2], 1,WAIT_TIME_MS) != 1 )
			return -1;
		i++;
	}
	*length = i+2;
	    
	if( (buf[2] != 0x30) && (buf[2] != 0x31) )
	{
		printf("packet error\n");
//		return -1;
	}
		    
	checksum = CalculateChecksum(buf, *length-1);
	
	if(checksum == buf[*length-1])
	{

#ifdef DEBUG_IR_API
		printf("Rx:");
		for(i=0;i<*length; i++)
		{
			printf("  %02x",buf[i]);
		}
		printf("\n");
#endif
	
		return 0;
	}
	else
		return -1;
}



/**
 * transmit_data() - transmit the ir command
 *
 * @tx_buf: the transmit data : command id and data
 * @cnt: the longth of txbuf
 * @rebuf: the value data of receive data for user, include chucksum
 * @relen: the length of rebuf
 * msec : wait for data msec(*ms)
 *
 * returns: the real transmit data count 
 *
 */
static int transmit_data( int fd, const unsigned char * tx_buf, int tx_len, unsigned char * rx_buf, int *rx_len, int msec )
{
#ifdef DEBUG_IR_API
	printf( "Enter:%s\n", __func__ );
#endif
	int ret, i, len,repeattx;
	unsigned char data;
	//BYTE buffer[10]={0x45,0x5a,0x01,0x09,0x81,0x01,0x01,0x22};
	unsigned char buffer[CMDLEN] = {0x45, 0x5a,0x0,};
	
	if((fd<=0)||(tx_buf == NULL)||(rx_buf == NULL)||(rx_len == NULL))
	{
		printf("parameter error\n");
		return -1;
	}
	
	len = tx_len + 4;
	buffer[2] = tx_buf[0];
	buffer[3] = len -1;
	for(i = 1; i< tx_len; i++)
		buffer[i+3] = tx_buf[i];	

		
	buffer[len -1]=CalculateChecksum(buffer,len-1);
	
	while( read_data(fd,&data, 1,2) > 0 )
		;/* clear serial FIFO */

	msleep(WAIT_TIME_BEFORE_TRIMIT);
   	ret = write(fd,buffer, len);

#ifdef DEBUG_IR_API
	printf("Tx:");
	for(i=0;i<ret; i++)
	{
		printf("  %02x",buffer[i]);
	}
	printf("\n");
#endif

	/*********receivedata process*********/
	repeattx = REPEAT_TX_CNT;
	
	ret = receive_data( fd, rx_buf, rx_len, msec );

	while( (repeattx !=0) && (ret == -1 ))
	{
		//printf("repeat tx\n");
		while( read_data(fd,&data, 1, msec) > 0 )
			;
		data = write(fd,buffer, len);
#ifdef DEBUG_IR_API
		printf("Tx:");
		for(i=0;i<data; i++)
		{
			printf("  %02x",buffer[i]);
		}
		printf("\n");
#endif
		repeattx--;
		ret = receive_data(fd,rx_buf,rx_len,msec);
	}

	return ret;
}

/**
 * IrTransmitStop() - stop ir taransmission
 *
 *
 */
int IrTransmitStop(int fd)
{
	unsigned char cmd = IR_CMD_STOP_TX ;
	int ret = -1;
	if( fd > 0 )
	{
		if( write(fd,&cmd, 1) == 1 )
			ret = 0;
	}
	return ret;
}

/*****************************
TRANSMIT PREPROGRAMMED IR CODE
dodeNum: code number
IrCodeSrc: the code number where store in the database

1:Internal IR code
2:Supplementary IR code
3:Extended IR code
4:Learned IR code

transType: transmission type, continous(0x00) and single(0x08)
keyid: key id

******************************/

int IrTransmitCode(int fd, WORD codeNum, BYTE keyID, BYTE IrCodeSrc, BYTE transType, BYTE deviceType)
{
	BYTE buffer[CMDLEN];
	int length;
	int idx = 0;
	int ret = -1;

	switch(IrCodeSrc)
	{
	case 0x4:	
		buffer[idx++] = IR_CMD_TX_LEARNED_CODE ;
		buffer[idx++] = 0x01+ transType*0x10;
		//buffer[idx++] = dodeNum%10;
		buffer[idx++] = codeNum;
		ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);
		break;
	
	case 0x01:
	case 0x02:
	case 0x03:
		buffer[idx++] = IR_CMD_TX_PREP_CODE	;
		buffer[idx++] = IrCodeSrc+ transType*0x10;
#if 1
        buffer[idx++] = deviceType;
        buffer[idx++] = (BYTE)(codeNum & 0xFF);
        buffer[idx++] = (BYTE)(codeNum >> 8);
#endif


#if 0
        buffer[idx++] = codeNum%10;
		//buffer[idx++] = deviceType;
		buffer[idx++] = (codeNum/10)/255;
		buffer[idx++] = (codeNum/10)%255;
#endif
        buffer[idx++] = keyID;
		ret = transmit_data(fd, buffer, idx, buffer, &length, WAIT_TIME_MS);	
        printf("transmit_data, idx:%d, length:%d, ret:%d\n", idx, length, ret);
		break;

	default:
		ret = -1;
	}
	return ret ;
}


/***********************************
GET KEY FLAG
transtype : ir transimission type
codenum:	code number or code location number
keyflag: Key Flag

length: the length of keyflag

***********************************/
int IrGetKeyflag( int fd, WORD dodeNum, BYTE *keyflag, int *length )
{
	unsigned char buffer[CMDLEN];
	int len = -1 ;
	int ret = -1 ;
	int idx = 0;
	
	if((fd<=0)||(keyflag == NULL)||(length ==NULL))
	{
		printf("parameter error\n");
		return -1;
	}
		
	buffer[idx++] = IR_CMD_GET_KEY_FLAG ;
	buffer[idx++] = dodeNum%10;
	buffer[idx++] = (dodeNum/10)/255;
	buffer[idx++] = (dodeNum/10)%255;
	ret = transmit_data(fd, buffer, idx, buffer, &len,WAIT_TIME_MS);

#ifdef DEBUG_IR_API
	printf("len = %d\n", len);
	for(idx = 0; idx<len; idx++)
		printf("  %x",buffer[idx]);
	printf("\n");

#endif
	
	for(idx = 0; idx< (len-4); idx++)
		keyflag[idx] = buffer[idx+3];

	*length = len-4;
		
	return ret;
}

/*************************************
LEARN IR CODE
location: IR Code Storage Location Assigned by Host
*************************************/
int IrLearnCode(int fd, BYTE location)
{
	BYTE buffer[CMDLEN] ;
	int ret = -1;
	int idx = 0 ;
	int length = 0 ;
	
	buffer[idx++] = IR_CMD_LEARN_CODE ;
	buffer[idx++] = location;
	ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);
	if(ret == SUCCESS)
	{
		ret = receive_data(fd, buffer, &idx, 16000);
	}

	return ret;
}


#if IR_VERSION > 0x09
int Ir_ReadE2prom( int fd, int location, unsigned char* data )
{
	int ret = -1 ;
	return -1;
}
#endif


/**
 * IrStoreToE2prom() - Store supplementary library to E2PROM
 *
 * @location: Upgrade IR Code Location
 * @data: IR Data
 * @datalength: Data Packet Number
 *
 */
int IrStoreToE2prom(int fd, BYTE location, BYTE *data)
{
	BYTE buffer[CMDLEN];
	int idx = 0, length,ret;
	int count = 0;
	int i = 0;
	if((fd<=0)||(data == NULL))
	{
		return ERROR;
	}
	//first
	buffer[idx++] = 0x07;
	buffer[idx++] = location;
	buffer[idx++] = 0x00;
	for( i=0; i<121;i++)
	{
		buffer[idx++] = data[count++];
	}		
	ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);
	if(ret == ERROR)
	{
		return ERROR;
	}

	//second
	idx = 0;
	buffer[idx++] = 0x07;
	buffer[idx++] = location;
	buffer[idx++] = 0x01;
	for(i=0; i<121;i++)
	{
		buffer[idx++] = data[count++];
	}		
	ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);

	if(ret == ERROR)
	{
		return ERROR;
	}

	//third
	idx = 0;
	buffer[idx++] = 0x07;
	buffer[idx++] = location;
	buffer[idx++] = 0x02;
	for(i=0; i<121;i++)
	{
		buffer[idx++] = data[count++];
	}
	ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);

	if(ret == ERROR)
	{
		return ERROR;
	}

	//fourth
	idx = 0;
	buffer[idx++] = 0x07;
	buffer[idx++] = location;
	buffer[idx++] = 0x03;
	for(i=0; i<121;i++)
	{
		buffer[idx++] = data[count++];
	}
	ret = transmit_data(fd, buffer, idx, buffer, &length,WAIT_TIME_MS);

	if(ret == ERROR)
	{
		return ERROR;
	}

	//fifth
	idx = 0;
	buffer[idx++] = IR_CMD_STORE ;
	buffer[idx++] = location;
	buffer[idx++] = 0x04;
	for(i=0; i<108;i++)
	{
		buffer[idx++] = data[count++];
	}
	ret = transmit_data(fd, buffer, idx, buffer, &length,1000);
	return ret;

}


/*  !function
 *  *****************************************************
 *  Name			: IrReadFromE2prom
 *
 *  Function       	: read supplemental IR data that stored in external EEPROM 
 *
 *  Input          	:fd:  serial port 
 *					 storageLoction(range:0~13): Location that you want to read 
 *					 data: Corresponding supplemental IR data
 *  Output        	:NULL
 *  
 *  Return        	:sucess: 0	failure: -1
 *
 *  Extern         	:NULL
 *
 *  Programmer   	:zhangning
 *
 *  Date          	:2010.12.24
 *
 *  ****************************************************
 */
int IrReadFromE2prom(int fd, BYTE storageLocation, BYTE *data)
{
	BYTE transmitBuffer[CMDLEN] = {0};
	BYTE receiveBuffer[CMDLEN] = {0};
	int transmitLength = -1;
	int receiveLength = -1;
	int dataLength = 0;
	int packageNum = -1;
	int ret = -1;
	int i = 0;
	int j = 0;

	/* validate parameters valide */
	assert(fd > 0);
	assert(storageLocation <= IR_STORE_MAX_LOCATION);
	assert(data != NULL);

	for	(packageNum = 0; packageNum < IR_PACKAGE_COUNT; packageNum++)
	{
		i = 0;
		bzero(transmitBuffer, CMDLEN);
		bzero(receiveBuffer, CMDLEN);

	/* prepare transmited data */
		transmitBuffer[i++] = IR_CMD_READ_DATA;
		transmitBuffer[i++] = storageLocation;
		transmitBuffer[i++] = packageNum;
		transmitLength = i;

	/* transmit package */
		ret = transmit_data(fd, transmitBuffer, transmitLength, 
							receiveBuffer, &receiveLength, WAIT_TIME_MS);
		if (-1 == ret)
		{
			printf("In Function: <IrReadFromE2prom>"
				   "transtmit first package error\n");
			return ERROR;
		}
		printf("receive length = %d \n", receiveLength - 4);

	/* get Supplemental IR Data */
		for (j = 0; j < (receiveLength - 4); j++)
		{
			data[dataLength++] = receiveBuffer[j + 3];
		}
	}
	
	return SUCCESS;
}

/***********************************
GET REVISION
version: This is revision information.
length: the length of version
***********************************/
int IrGetVersion(int fd, BYTE *version, BYTE *length)
{
	unsigned char buffer[CMDLEN];
	int idx = 0;
	int ret = -1;
	int len = 0;
	
	if((fd<=0)||(version == NULL)||(length == NULL))
	{
		return ERROR;
	}
	
	buffer[idx++] = IR_CMD_GET_REVISION ;
	ret = transmit_data(fd, buffer, idx, buffer, &len,WAIT_TIME_MS);
	
	for(idx = 0; idx<(len-4); idx++)
		version[idx] = buffer[idx+3];
	*length = len-4;
	return ret;
}

/***********************************
MASTER CLEAR
***********************************/
int IrMasterClear(int fd)
{
	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_MASTER_CLEAR ;
	return transmit_data(fd, buffer, idx, buffer, &idx,WAIT_TIME_MS);
}

/**********************************
STANDBY
**********************************/
int IrStandby(int fd)
{
	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_STANDBY ;
	return transmit_data(fd, buffer, idx, buffer, &idx,WAIT_TIME_MS);
}

/**
WAKE UP
*********************************/
int IrWakeup(int fd)
{
	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_WAKEUP ;
	return transmit_data(fd, buffer, idx, buffer, &idx,WAIT_TIME_MS);
}

/******************************
SET GPIO
gpConfig: GPIO Configuration
Bit[0:6] of this GPIO Configuration byte correspond to GPIO[0:6].
******************************/
int IrSetGpio(int fd, BYTE gpConfig)
{
	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_SET_GPIO	;
	buffer[idx++] = gpConfig;
	return transmit_data(fd, buffer, idx,buffer, &idx,WAIT_TIME_MS);
}

/******************************
READ GPIO CONFIGURATION
gpconfig:
 Bit[0:6] of this GPIO Configuration byte correspond to GPIO[0:6].
¡°0¡± configures the corresponding GPIO as INPUT
¡°1¡± configures the corresponding GPIO as OUTPUT
******************************/
int IrReadGgpioConfig(int fd, BYTE *gpconfig)
{
	BYTE buffer[CMDLEN];
	int idx = 0,len, ret;
	if((fd<=0)||(gpconfig == NULL))
		return ERROR;
	buffer[idx++] = IR_CMD_READ_GPIO_CONF ;
	ret = transmit_data(fd, buffer, idx, buffer, &len,WAIT_TIME_MS);
	*gpconfig = buffer[3];
	return ret;	
}

/*****************************
WRITE TO GPIO
data: Bit[6:0] of this DATA byte correspond to GPIO[6:0], and ¡®0¡¯ in a bit is Low, ¡®1¡¯ in a bit is High.
Bit [7] is reserved.
*****************************/
int IrWriteToGgpio(int fd, BYTE data)
{

	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_WRITE_GPIO ;
	buffer[idx++] = data;
	return transmit_data(fd, buffer, idx,buffer, &idx,WAIT_TIME_MS);
}
/******************************
READ GPIO
******************************/

int IrReadGgpio(int fd, BYTE *gpconfig)
{
	BYTE buffer[CMDLEN];
	int idx = 0, ret, len;
	if((fd<=0)||(gpconfig == NULL))
		return ERROR;
	buffer[idx++] = IR_CMD_READ_GPIO;
	ret = transmit_data(fd, buffer, idx, buffer, &len,WAIT_TIME_MS);
	*gpconfig = buffer[3];
	return ret;
}

/**
 * IrBackLightCtrl() -  BACKLIGHT CONTROL
 *
 * @fd - file descript.
 * @state - On(1)/Off(0) 
 *
 * 0x5F ON (control output high)
 * 0x50 OFF (control output low)
 */
int IrBackLightCtrl( int fd, int state )
{	
	BYTE buffer[CMDLEN];
	int idx = 0;
	if(fd<=0)
		return ERROR;
	buffer[idx++] = IR_CMD_BACK_LIGHT_CTRL ;
	buffer[idx++] = state ? 0x5f:0x50 ;
	return transmit_data(fd, buffer, idx,buffer, &idx,WAIT_TIME_MS);
}

/*  !function
 *  *****************************************************
 *  Name			: Boost_Set
 *
 *  Function       	: set boost 
 *
 *  Input          	: state: 0: set boost off
 *  						 1: set boost on
 *
 *  Output        	: null
 *  
 *  Return        	: sucess: 0	failure: -1
 *
 *  Extern         	: NULL
 *
 *  Programmer   	: zhangning
 *
 *  Date          	: 2011.6.24
 *
 *  ****************************************************
 */
int Boost_Set( int state )
{
	int fd = 0;
	int ret = 0;
	int boost_state = -1;
	
	/* check paramters */
	if ((state != 1) && (state != 0))
	{
		printf("Paramters error\n");
		return ERROR;
	}

	/* open serial port */
	fd = serial_open("/dev/s3c2410_serial1", 19200);
    if (fd < 0)
    {   
        printf("open serial port error\n");
        return ERROR;
    }   

	/* set all gpio are output status */
    ret = IrSetGpio(fd, 0xff);
    if (ret < 0)
    {   
        printf("set gpio output error\n");
        return ERROR;
    }   

	if (state == 1)
	{
		boost_state = 0x40;
	}
	else if (state == 0)
	{
		boost_state = 0x00;
	}
	else
	{
		return ERROR;
	}

	/* set bit[6] gpio 's status */
    ret = IrWriteToGgpio(fd, boost_state);
    if (ret < 0)
    {   
        return ERROR;
    }   

	return SUCCESS;
} 

/*  !function
 *  *****************************************************
 *  Name			: Boost_Get
 *
 *  Function       	: get boost status
 *
 *  Input          	: null
 *
 *  Output        	: null
 *  
 *  Return        	: sucess: 0: Boost off
 *  						 64: Boost on	
 *  				  failure: -1
 *
 *  Extern         	: NULL
 *
 *  Programmer   	: zhangning
 *
 *  Date          	: 2011.6.24
 *
 *  ****************************************************
*/
int Boost_Get()
{
	int fd = 0;
	int ret = 0;
	unsigned char gpioStatus = 0;

	/* open serial port */
	fd = serial_open("/dev/s3c2410_serial1", 19200);
    if (fd < 0)
    {   
        printf("open serial port error\n");
        return ERROR;
    } 
  
	/* read gpio status */
	ret = IrReadGgpio(fd, &gpioStatus);
	if (ret < 0)
	{
		return ERROR;
	}

	return gpioStatus;
}

/*  !function
 *  *****************************************************
 *  Name			: IrGetLearnData
 *
 *  Function       	: Get learn data that was stored at corresponding location from RT300
 *
 *  Input          	:fd:  serial port 
 *					 storageLoction:The location of learn data
 *
 *  Output        	:learnData: learn data is generated by RT300
 *					 dataLength: learn data's length 
 *  
 *  Return        	:sucess: 0	failure: -1
 *
 *  Extern         	:NULL
 *
 *  Programmer   	:zhangning
 *
 *  Date          	:2010.12.21
 *
 *  ****************************************************
 */
int IrGetLearnData(int fd, int storageLoction, BYTE *learnData, int *dataLength)
{
	BYTE transmitBuffer[CMDLEN] = {0};
	BYTE receiveBuffer[CMDLEN] = {0};
	int transmitLength = -1;
	int receiveLength = -1;
	int ret = -1;
	int i = 0;
	int j = 0;
	
	/* validate parameters valide */
	assert (fd > 0);
	assert (storageLoction >= 0 
			&& storageLoction < IR_LEARN_MAX_LOCATION);
	assert (learnData != NULL);
	
	/* prepare transmited data */
	transmitBuffer[i++] = IR_CMD_GET_LEARN_DATA;
	transmitBuffer[i++] = storageLoction;
	
	transmitLength = i;
#ifdef GET_LEARN_DEBUG
	for (i = 0; i < transmitLength; i ++)
	{
		printf("transmitBuffer[%d] = %02x\n", i, transmitBuffer[i]);
	}
#endif
	
	/* transmit prepared data to IR_Engine */
	ret = transmit_data(fd, transmitBuffer, transmitLength, 
						receiveBuffer, &receiveLength, WAIT_TIME_MS);
	if (-1 == ret)
	{
		printf("In Function: <IrGetLearnData>"
			   "transtmit IR data error\n");
		
		return ERROR;
	}
	
	/* get data from IR_Engine */
	for (j = 0; j < (receiveLength - 4); j++)
	{
		learnData[j] = receiveBuffer[j + 3];
	}

#ifdef GET_LEARN_DEBUG
	for (i = 0; i < receiveLength; i ++)
	{
		printf("receiveBuffer[%d] = %02x\n", i, receiveBuffer[i]);
	}
#endif
	if (j == IR_LEARN_DATA_LENGTH)
	{
		*dataLength = j;
	}
	else
	{
		printf("In Function: <IrGetLearnData>"
			   "get learn data's length error\n");
		
		return ERROR;
	}

	return SUCCESS;
}

/*  !function
 *  *****************************************************
 *  Name			: IrStorageLearnData
 *
 *  Function       	: Storage learn data to RT300
 *
 *  Input          	:fd:  serial port 
 *					 storageLoction(range:0~99): Location that you want to store learn data 
 *					 learnData: Learn data that you want to store RT300
 *					 dataLength: learn data's length
 *  Output        	:NULL
 *  
 *  Return        	:sucess: 0	failure: -1
 *
 *  Extern         	:NULL
 *
 *  Programmer   	:zhangning
 *
 *  Date          	:2010.12.21
 *
 *  ****************************************************
 */
int IrStorageLearnData(int fd, int storageLoction, BYTE *learnData, int dataLength)
{
	BYTE transmitBuffer[CMDLEN] = {0};
	BYTE receiveBuffer[CMDLEN] = {0};
	int transmitLength = -1;
	int receiveLength = -1;
	int ret = -1;
	int i = 0;
	int j = 0;
	
	/* validate parameters valide */
	assert(fd > 0);
	assert(storageLoction >= 0 
			&& storageLoction < IR_LEARN_MAX_LOCATION);
	assert(learnData != NULL);
	assert(dataLength = IR_LEARN_DATA_LENGTH);
	
	/* prepare transmited data */
	transmitBuffer[i++] = IR_CMD_SET_LEARN_DATA;
	transmitBuffer[i++] = storageLoction;
	for (j = 0; j < dataLength; j++)
	{
		transmitBuffer[i++] = learnData[j];
	}
	transmitLength = i;

#ifdef STORE_LEARN_DEBUG
	for (i = 0; i < transmitLength; i ++)
	{
		printf("transmitBuffer[%d] = %02x\n", i, transmitBuffer[i]);
	}
#endif
	
	/* transmit prepared data to IR_Engine */
	ret = transmit_data(fd, transmitBuffer, transmitLength, 
						receiveBuffer, &receiveLength, WAIT_TIME_MS);
	if (-1 == ret)
	{
		printf("In Function: <IrStoreLearnData>"
			   "transtmit IR data error\n");
		
		return ERROR;
	}

	return SUCCESS;
}

