/**
This software package supports the communications with SII thermal printers.
Copyright ( C ) 2007 by Seiko Instruments Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or ( at your option ) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <termios.h>
#include "lp.h"
#include <sys/ioctl.h>
//#include <aio.h>

#include "sii_api.h"



#define TRUE							1
#define FALSE							0

// identifier of status data
#define AUTO_STATUS_HEAD_MASK			0x90
#define AUTO_STATUS_HEAD_DATA			0x10
#define AUTO_STATUS_OTHER_MASK			0x90
#define AUTO_STATUS_OTHER_DATA			0x00

// Defoult timeout
#define TIMEOUT_SEC						0
#define TIMEOUT_USEC_LONG				500*1000		// 500 ms
#define TIMEOUT_USEC_SHRT				50*1000			// 50 ms

#define WRITE_TIMEOUT_SEC				5				// 30 s
//#define WRITE_DIV_TIMEOUT_SEC			5				// 5 s
#define WRITE_TIMEOUT_NSEC				0
#define WRITE_TIMEOUT_USEC				0
//#define TIMEOUT_LOOP_COUNT				(WRITE_TIMEOUT_SEC/WRITE_DIV_TIMEOUT_SEC)

// Max data size
#define MAX_WRITE_BYTES					1024*1024*1024

// Type of interface
#define	DEV_USB							0			// device type
#define DEV_SER							1
#define DEV_PAR							2
#define	PORT_USB						"/dev/usb"	// device port name
#define PORT_SER						"/dev/ttyS"
#define PORT_PAR						"/dev/lp"
#define PORT_PAR2						"/dev/par"


#define AUTO_STATUS_DATA_SIZE			4
#define GET_STATUS_RETRY_COUNT			10
#define GET_STATUS_WAIT_TIME			(10000)		// 10ms


// device info structure
typedef struct
{
	int					nDeviceType;								// Device type
	int					nDeviceFid;									// Device descriptor
	unsigned char		byAutoStatusBuf[AUTO_STATUS_DATA_SIZE]; 	// Auto status response data
	unsigned char		byAutoStatusIndex;							// Index of auto status data
	CALLBACK_FUNC		pfnUsrDefFunc;								// User defined function for call back status
} DEV_INFO, *PDEV_INFO;


// Macro
#define is_null( val )							( val == NULL )

#define OPEN_ERR()								\
{												\
	if( phDevInfo != NULL )						\
	{											\
		free( phDevInfo );						\
	}											\
	if( errno == EBUSY )						\
	{											\
		return ( ERR_SII_API_DEV_EBUSY );		\
	}											\
	else if( errno == EACCES )					\
	{											\
		return ( ERR_SII_API_DEV_EACCES );		\
	}											\
	else if( errno == ENXIO )					\
	{											\
		return ( ERR_SII_API_DEV_ENXIO );		\
	}											\
	else 										\
	{											\
		return ( ERR_SII_API_DIS_OPEN );		\
	}											\
}



// open device function
int open_device_usb( PDEV_INFO, char * );
int open_device_ser( PDEV_INFO, char * );
int open_device_par( PDEV_INFO, char * );
static int ( *open_device[] )( PDEV_INFO, char * ) =
{
	open_device_usb,
	open_device_ser,
	open_device_par
};

// write device function
static int write_device(
			PDEV_INFO,
			unsigned char *,
			size_t,
			size_t *,
			struct timeval * );

// read device function
static int read_device(
			PDEV_INFO,
			unsigned char *,
			size_t,
			size_t *,
			struct timeval * );

// reset device function
static int reset_device_usb( PDEV_INFO );
//static int reset_device_ser( PDEV_INFO );
//static int reset_device_par( PDEV_INFO );
static int ( *reset_device[] )( PDEV_INFO ) =
{
	reset_device_usb,
	//reset_device_ser,
	//reset_device_par
};








/*******************************************

Global function

********************************************/

/**
 * open device
 *
 * @param[ out ] hSiiDevice		handle to device object
 * @param[ in ] pszPath			device path
 * @retval						return values of function result
 *
 */
int
sii_api_open_device(
	SIIAPIHANDLE		*hSiiDevice,
	char				*pszPath )
{
	PDEV_INFO			*phDevInfo = (PDEV_INFO*)hSiiDevice;

	if( is_null( hSiiDevice ) || is_null( pszPath ) )
	{
		return ( ERR_SII_API_NULL );
	}

	if( ( *phDevInfo = malloc( sizeof( DEV_INFO ) ) ) == NULL )
	{
		return ( ERR_SII_API_MALLOC );
	}

	memset( *phDevInfo, 0, sizeof( DEV_INFO ) );

	if( !strncmp( pszPath, PORT_USB, strlen( PORT_USB ) ) )
	{
		return open_device_usb( *phDevInfo, pszPath );
	}
	else if( !strncmp( pszPath, PORT_SER, strlen( PORT_SER ) ) )
	{
		return open_device_ser( *phDevInfo, pszPath );
	}
	else if( !strncmp( pszPath, PORT_PAR, strlen( PORT_PAR ) ) ||
			 !strncmp( pszPath, PORT_PAR2, strlen( PORT_PAR2 ) ) )
	{
		return open_device_par( *phDevInfo, pszPath );
	}
	else
	{
		free( *phDevInfo );
		return ( ERR_SII_API_DIS_OPEN );
	}
}


/**
 * close device
 *
 * @param[ out ] hSiiDevice		handle to device object
 * @retval						return values of function result
 */
int
sii_api_close_device(
	SIIAPIHANDLE		hSiiDevice )
{
	PDEV_INFO			phDevInfo = (PDEV_INFO)hSiiDevice;

	if( is_null( hSiiDevice ) )
	{
		return ( ERR_SII_API_NULL );
	}

	if( phDevInfo->nDeviceType == DEV_SER )
	{
		// Exclusive control OFF
		ioctl( phDevInfo->nDeviceFid, TIOCNXCL );
	}

	if( close( phDevInfo->nDeviceFid ) < 0 )
	{
		return ( ERR_SII_API_DIS_CLOSE );
	}

	free( phDevInfo );
	phDevInfo = NULL;
	return ( SUCC_SII_API );
}

/**
 * write device
 *
 * @param[ in ] hSiiDevice		handle to device object
 * @param[ in ] pbyCmd  		array of printer data
 * @param[ in ] tagCmdSize		size of array
 * @param[ out ] pSize			bytes written to printer
 * @retval						return values of function result
 */
int
sii_api_write_device(
	SIIAPIHANDLE		hSiiDevice,
	unsigned char		*pbyCmd,
	size_t				tagCmdSize,
	size_t				*pSize)
{
	PDEV_INFO			phDevInfo = (PDEV_INFO)hSiiDevice;
	unsigned char		byTmpBuf[512];
	int					nRetVal=0;

	if( is_null( hSiiDevice ) || is_null( pbyCmd ) || is_null( pSize ) )
	{
		return ( ERR_SII_API_NULL );
	}

	if( tagCmdSize <= 0 )
	{
		return ( ERR_SII_API_RANGE );
	}

       

        struct timeval		tagTimeout;
        tagTimeout.tv_sec = 3;
	tagTimeout.tv_usec = TIMEOUT_USEC_LONG; 

	if( phDevInfo->pfnUsrDefFunc != NULL )
	{
		  
                  nRetVal = read_device(
				phDevInfo,
				byTmpBuf,
				sizeof(byTmpBuf),
				NULL,
				&tagTimeout );
	}


       
        
	if( nRetVal >= 0 )
	{
		nRetVal = write_device(
				phDevInfo,
				pbyCmd,
				tagCmdSize,
				pSize,
				&tagTimeout );
	}

	return nRetVal;
}


/**
 * read device
 *
 * @param[ in ] hSiiDevice		handle to device object
 * @param[ in ] pbyResp			data buffer
 * @param[ in ] tagRespSize		size of data buffer
 * @param[ out ] pSize			bytes received
 * @retval						return values of function result
 */
int
sii_api_read_device(
	SIIAPIHANDLE		hSiiDevice,
	unsigned char		*pbyResp,
	size_t				tagRespSize,
	size_t				*pSize )
{
	PDEV_INFO			phDevInfo = (PDEV_INFO)hSiiDevice;
	struct timeval		tagTimeout;

	if( is_null( hSiiDevice ) || is_null( pbyResp ) || is_null( pSize ) )
	{
		return ( ERR_SII_API_NULL );
	}

	if( tagRespSize <= 0 )
	{
		return ( ERR_SII_API_RANGE );
	}

	tagTimeout.tv_sec = TIMEOUT_SEC;
	tagTimeout.tv_usec = TIMEOUT_USEC_LONG;

	return read_device(
				phDevInfo,
				pbyResp,
				tagRespSize,
				pSize,
				&tagTimeout );
}

/**
 * get status
 *
 * @param[ in ] hSiiDevice		handle to device object
 * @param[ out ] plResp			responce data area
 * @retval						return values of function result
 *
 */
int
sii_api_get_status(
	SIIAPIHANDLE		hSiiDevice,
	unsigned long		*plResp)
{
	PDEV_INFO			phDevInfo = (PDEV_INFO)hSiiDevice;
	int					nRetVal;
	int					i;
	unsigned char		byStaCmd[] = { 0x1d, 0x61, 0xFF };
	unsigned char		byTmpBuf[512];
	struct timeval		tagTimeout;
	size_t				tagSize;

	if( is_null( hSiiDevice ) || is_null( plResp ) )
	{
		return ( ERR_SII_API_NULL );
	}

	tagTimeout.tv_sec = TIMEOUT_SEC;
	tagTimeout.tv_usec = TIMEOUT_USEC_LONG;

	nRetVal = write_device(
			phDevInfo,
			(unsigned char*)byStaCmd,
			sizeof( byStaCmd ),
			NULL,
			&tagTimeout );

	if( nRetVal < 0 )
	{
		return nRetVal;
	}

	for( i = 0; i < 1/*GET_STATUS_RETRY_COUNT*/; i ++ )
	{

		nRetVal = read_device(
				phDevInfo,
				byTmpBuf,
				sizeof(byTmpBuf),
				&tagSize,
				&tagTimeout );

		if( nRetVal < 0 )
		{
			return nRetVal;
		}

		if( tagSize >= sizeof( unsigned long ) )
		{
			break;
		}
		usleep( GET_STATUS_WAIT_TIME );
	}

	if( *(phDevInfo->byAutoStatusBuf) == 0x00 )
	{
		return ( ERR_SII_API_NO_DATA );
	}

	*plResp = (unsigned long)(
				phDevInfo->byAutoStatusBuf[0] << 0	|
				phDevInfo->byAutoStatusBuf[1] << 8	|
				phDevInfo->byAutoStatusBuf[2] << 16	|
				phDevInfo->byAutoStatusBuf[3] << 24	);

	return ( SUCC_SII_API );
}



/**
 * reset device
 *
 * @param[ in ] hSiiDevice		handle to device object
 * @retval						return values of function result
 */
int
sii_api_reset_device(
	SIIAPIHANDLE		hSiiDevice )
{
	PDEV_INFO			phDevInfo = (PDEV_INFO)hSiiDevice;

	if( is_null( hSiiDevice ) )
	{
		return ( ERR_SII_API_NULL );
	}

	return reset_device[ phDevInfo->nDeviceType ]( phDevInfo );
}



/*******************************************

Internal function

********************************************/



/**
 * open USB device interface
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @param[ in ] pszPath			device path
 * @retval						return values of function result
 *
*/
int
open_device_usb(
	PDEV_INFO			phDevInfo,
	char				*pszPath )
{
	int					nFd;
	char				pszDeviceId[1024];

	if( ( nFd = open( pszPath, O_RDWR | O_EXCL ) ) < 0 )
	{
		OPEN_ERR();
	}

	memset( pszDeviceId, 0, sizeof(pszDeviceId) );

	// get printer device id
	if( ioctl( nFd, _IOC(_IOC_READ, 'P', 1, sizeof(pszDeviceId) ), pszDeviceId ) )
	{
		OPEN_ERR();
	}

	// check Vendor
	if( strstr( pszDeviceId+2, "MFG:SII;" ) == 0 )
	{
		OPEN_ERR();
	}


	phDevInfo->nDeviceType	= DEV_USB;
	phDevInfo->nDeviceFid	= nFd;

	return ( SUCC_SII_API );
}

/**
 * open Serial device interface
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @param[ in ] pszPath			device path
 * @retval						return values of function result
 *
*/
int
open_device_ser(
	PDEV_INFO			phDevInfo,
	char				*pszPath )
{
	int					nFd;
	struct termios		tagOpts;
	struct termios		tagOrigOpts;

	if( ( nFd = open( pszPath, O_RDWR | O_NOCTTY | O_EXCL | O_NONBLOCK ) ) < 0 )
	{
		OPEN_ERR();
	}

	phDevInfo->nDeviceType	= DEV_SER;
	phDevInfo->nDeviceFid	= nFd;
	tcgetattr( nFd, &tagOpts );
	tcgetattr( nFd, &tagOrigOpts );
	tagOpts.c_lflag &= ~( ICANON | ECHO | ISIG );
	tcsetattr( nFd, TCSANOW, &tagOpts );

	// Exclusive control ON
	if( ioctl( nFd, TIOCEXCL ) != 0 )
	{
		OPEN_ERR();
	}

	return ( SUCC_SII_API );
}


/**
 * open Parallel device interface
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @param[ in ] pszPath			device path
 * @retval						return values of function result
 *
 */
int
open_device_par(
	PDEV_INFO			phDevInfo,
	char				*pszPath )
{
	int					nFd;

	if( ( nFd = open( pszPath, O_RDWR | O_EXCL | O_NONBLOCK ) ) < 0 )
	{
		OPEN_ERR();
	}

	phDevInfo->nDeviceType	= DEV_PAR;
	phDevInfo->nDeviceFid	= nFd;

	return ( SUCC_SII_API );
}

// write device
/**
 * write USB or Serial or Parallel device
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @param[ in ] pbyCmd  		array of printer data
 * @param[ in ] tagCmdSize		size of array
 * @param[ out ] pSize			bytes written to printer
 * @param[ in ] pTimeout		writing timeout value
 * @retval						return values of function result
 *
 */
static int
write_device(
	PDEV_INFO			phDevInfo,
	unsigned char		*pbyCmd,
	size_t				tagCmdSize,
	size_t				*pSize,
	struct timeval		*pTimeout )
{
	int					nDeviceFid;
	int					nRetVal = 0;
	size_t				tagWriteBytes = 0;
	fd_set				tagOutput;
	struct timeval		tagTimeout;
	int					nRetBytes = 0;
	int					i;
	struct timespec		tagTimespec;
	//struct aiocb		tagAioWrite;
	//const struct aiocb 	*plistAiocb[1];

	if( pSize != NULL )
	{
		*pSize = 0;
	}

	if( pTimeout != NULL )
	{
	     	
              memcpy( &tagTimeout, pTimeout, sizeof( struct timeval ) );
	}
	else
	{
		tagTimeout.tv_sec = WRITE_TIMEOUT_SEC;
		tagTimeout.tv_usec = WRITE_TIMEOUT_USEC;
	}

	if( tagCmdSize < MAX_WRITE_BYTES )
	{
		tagWriteBytes = tagCmdSize;
	}
	else
	{
		tagWriteBytes = MAX_WRITE_BYTES;
	}

	nDeviceFid = phDevInfo->nDeviceFid;
	FD_ZERO( &tagOutput );
	FD_SET( nDeviceFid, &tagOutput );
	 
        nRetVal = select(
				nDeviceFid + 1,
				NULL,
				&tagOutput,
				NULL,
				&tagTimeout );
	if( nRetVal < 0 )
	{
		return ( ERR_SII_API_SELECT );
	}
	else if( nRetVal == 0 )
	{
		return ( ERR_SII_API_TIMEOUT );
	}
     
	if( FD_ISSET( nDeviceFid, &tagOutput ) )
	{
	        
	    int res = write(nDeviceFid, pbyCmd, tagWriteBytes);
	    return ( SUCC_SII_API );
	}
      
}

/**
 * read device and analize rec data
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @param[ in ] pbyResp			data buffer
 * @param[ in ] tagRespSize		size of data buffer
 * @param[ out ] pSize			bytes received
 * @retval						return values of function result
 *
 */
static int
read_device(
	PDEV_INFO			phDevInfo,
	unsigned char		*pbyResp,
	size_t				tagRespSize,
	size_t				*pSize,
	struct timeval		*pTimeout )
{
	
        
        unsigned char		byLastAutoStatus[ AUTO_STATUS_DATA_SIZE ];
	unsigned char		pbyTmpBuf[ 512 ];
	int					nSelRet = 0;
	int					nDeviceFid = 0;
	int					nEndFlag = FALSE;
	int					nIndex;
	fd_set				tagInput;
	ssize_t				tagBytes = 0;
	size_t				tagReadBytes;
	size_t				tagSize = 0;
	struct timeval		tagTimeout;
	struct timeval		tagCurrentTime;
	struct timeval		tagStartTime;
	struct timeval		tagPassTime;

	if( tagRespSize > sizeof( pbyTmpBuf ) )
	{
		tagReadBytes = sizeof( pbyTmpBuf );
	}
	else if( tagRespSize > 0 )
	{
		tagReadBytes = tagRespSize;
	}
	else
	{
		return ( ERR_SII_API_RANGE );
	}

	memset( pbyResp, 0, tagRespSize );

	if( pSize != NULL )
	{
		*pSize = 0;
	}

	memcpy( byLastAutoStatus, phDevInfo->byAutoStatusBuf, AUTO_STATUS_DATA_SIZE );


       

	if( pTimeout != NULL )
	{
		memcpy( &tagTimeout, pTimeout, sizeof( struct timeval ) );
	}
	else
	{
		tagTimeout.tv_sec = TIMEOUT_SEC;
		tagTimeout.tv_usec = TIMEOUT_USEC_SHRT;
	}

	nDeviceFid = phDevInfo->nDeviceFid;

	// get start time ( for timeout )
	gettimeofday( &tagStartTime, NULL );

	// read data
        int acc = 0;
	for(acc=0;acc<1000;acc++)
	{
	
               
                FD_ZERO( &tagInput );
		FD_SET( nDeviceFid, &tagInput );
		nSelRet = select(
					nDeviceFid + 1,
					&tagInput,
					NULL,
					NULL,
					&tagTimeout );
		if( nSelRet < 0 )
		{
			return ( ERR_SII_API_SELECT );
		}
		else if( nSelRet == 0 )
		{
			break;
		}

		if( FD_ISSET( nDeviceFid, &tagInput ) )
		{
			memset( pbyTmpBuf, 0, sizeof( pbyTmpBuf ) );
			tagBytes = read(
						nDeviceFid,
						pbyTmpBuf,
						tagReadBytes );
			if( tagBytes < 0 )
			{
				if( phDevInfo->nDeviceType != DEV_PAR )
				{
					return ( ERR_SII_API_DEV_ACCESS );
				}

				if( errno == EIO )
				{
					return ( ERR_SII_API_DEV_ACCESS );
				}
				tagBytes = 0;
			}

			for( nIndex = 0; nIndex < tagBytes; nIndex++ )
			{
				if( ( pbyTmpBuf[ nIndex ] & AUTO_STATUS_HEAD_MASK ) == AUTO_STATUS_HEAD_DATA )
				{
					phDevInfo->byAutoStatusIndex = 0;
					phDevInfo->byAutoStatusBuf[ phDevInfo->byAutoStatusIndex ] = pbyTmpBuf[ nIndex ];
				}
				else if(
					( phDevInfo->byAutoStatusIndex < AUTO_STATUS_DATA_SIZE -1 )	&&
					( ( pbyTmpBuf[ nIndex ] & AUTO_STATUS_OTHER_MASK ) == AUTO_STATUS_OTHER_DATA ) )
				{
					phDevInfo->byAutoStatusIndex++;
					phDevInfo->byAutoStatusBuf[ phDevInfo->byAutoStatusIndex ] = pbyTmpBuf[ nIndex ];

					if( phDevInfo->byAutoStatusIndex == ( AUTO_STATUS_DATA_SIZE -1 ) )
					{
						// check auto status response
						phDevInfo->byAutoStatusIndex++;
						if(
							( phDevInfo->pfnUsrDefFunc != NULL)	&&
							( memcmp(
								phDevInfo->byAutoStatusBuf,
								byLastAutoStatus,
								AUTO_STATUS_DATA_SIZE ) != 0 ) )
						{
							memcpy( byLastAutoStatus, phDevInfo->byAutoStatusBuf, AUTO_STATUS_DATA_SIZE );
							phDevInfo->pfnUsrDefFunc( (unsigned long)(
								phDevInfo->byAutoStatusBuf[0] << 0	|
								phDevInfo->byAutoStatusBuf[1] << 8	|
								phDevInfo->byAutoStatusBuf[2] << 16	|
								phDevInfo->byAutoStatusBuf[3] << 24	) );	//call back status
						}
					}
				}

				nEndFlag = TRUE;
				if( ( pbyTmpBuf[ nIndex ] != 0xff )	&&
					( pbyTmpBuf[ nIndex ] != 0x11 )	&&
					( pbyTmpBuf[ nIndex ] != 0x13 ) )
				{
					if( tagReadBytes != 0 )
					{
						pbyResp[ tagSize++ ] = pbyTmpBuf[ nIndex ];
						if( tagReadBytes-- > 0 )
						{
							nEndFlag = FALSE;
						}
					}
				}
			}
		}

		/// check timeout
		gettimeofday( &tagCurrentTime,NULL );
		timersub( &tagCurrentTime, &tagStartTime, &tagPassTime );
		if( timercmp( &tagTimeout, &tagPassTime, < ) )
		{
			break;				// Timeout
		}

		if( ( tagBytes == 0 ) && ( tagSize != 0 ) )
		{
			break;				// End of data
		}

		if( nEndFlag == TRUE )
		{
			break;					// End flag was set
		}
	}

	if( pSize != NULL )
	{
		*pSize = tagSize;
	}

	return ( SUCC_SII_API );
}




// reset device
/**
 * reset USB device
 *
 * @param[ in ] phDevInfo		pointer of device info
 * @retval						return values of function result
 *
 */
static int
reset_device_usb(
	PDEV_INFO			phDevInfo )
{
	if( ioctl( phDevInfo->nDeviceFid, _IOC( _IOC_NONE, 'P', 7, 0 ), 0 ) < 0 )
	{
		return ( ERR_SII_API_RESET );
	}

	return( SUCC_SII_API );
}


