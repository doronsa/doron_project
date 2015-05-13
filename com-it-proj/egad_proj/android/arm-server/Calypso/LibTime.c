

///////////////////////////////////////////////////////////////////////////////////////
//
//     Abstract:
//         Extention to the time.h functions
//
//
///////////////////////////////////////////////////////////////////////////////////////

#include <Core.h>
#ifdef CORE_SUPPORT_LIBTIME


#ifdef WIN32
	#define win_or_linux
#endif
#if linux
	#define win_or_linux
#endif


#ifdef WIN32
#pragma warning(disable : 4996) // CRT Secure - off
#endif

extern int  os_gettime(struct tm *t); // 0 fail  1 success, see OS_GLOBAL_TIME for results
extern void os_settime(struct tm *t);

////////////////////////////////////////////////////////////////////////////////////
//
// Defines
//
////////////////////////////////////////////////////////////////////////////////////

#define SECONDS_IN_HOUR             3600
#define SECONDS_IN_MINUTE           60
#define SECONDS_IN_DAY              86400                // (24*SECONDS_IN_HOUR)
#define SECONDS_IN_REGULAR_YEAR     31536000
#define SECONDS_IN_LEAP_YEAR        31622400
#define IS_LEAP_YAER(i) (i%4 == 0)
#define TIME_ZONE_OFFSET_SECONDS    7200
#define FEBRUARY                     2
#define GET_NUM_OF_SEC_IN_YEAR(Year) (IS_LEAP_YAER(Year)?SECONDS_IN_LEAP_YEAR:SECONDS_IN_REGULAR_YEAR )
#define GET_NUM_OF_SEC_IN_MONTH(year,month) ((IS_LEAP_YAER(year) && month == FEBRUARY)? ((long)(uc_ArrDaysInMonth[month])*(long)SECONDS_IN_DAY+SECONDS_IN_DAY):((long)(uc_ArrDaysInMonth[month])*(long)SECONDS_IN_DAY))

typedef struct 
{
	unsigned int  ui_Year;
	unsigned char uc_StartMonth;    //  Month.		[1-12] 
	unsigned char uc_StartDay;		//  Day.		[1-31] 
	unsigned char uc_StartHour;		//  Hours.		[0-23] 
	unsigned char uc_EndMonth;		//  Month.		[1-12] 
	unsigned char uc_EndDay;		//  Day.		[1-31]
	unsigned char uc_EndHour;		//  Hours.		[0-23] 
	
	
} st_IsrDSTTable;


static const st_IsrDSTTable IsrDSTTable[] = {   {2009,3,27,3,9,27,1},
{2010,3,26,2,9,12,2},    
{2011,4,1,2,10,2,2},
{2012,3,30,2,9,23,2},
{0,0,0,0,0,0,0} };

////////////////////////////////////////////////////////////////////////////////////
//
// Globals
//
////////////////////////////////////////////////////////////////////////////////////


static const unsigned char uc_ArrDaysInMonth[] = {0,31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30,31};

////////////////////////////////////////////////////////////////////////////////////
//
// Internal functions
//
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_IsInputTimeStValid  
// Description: Check user input time
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

static int b_IsInputTimeStValid (const st_Time *p)
{
	// Basic check
	if(! 
		(p->ui_Year>=2000 && p->ui_Year<=2060) &&       //  Year    : for example 20000 
		( (p->uc_Month>=1) && (p->uc_Month<=12) ) &&    //  Month.  [1-12] 
		( (p->uc_Hour<=23) )  &&                        //  Hours.  [0-23] 
		( (p->uc_Minute<=59) )&&                        //  Minutes [0-59] 
		( (p->uc_Second<=59) )                          //  Seconds [0-59] 
		) 
		return 0;
	
	// Check special day 29 of febuary in a leap year 
	if( (p->uc_Month == FEBRUARY) && (IS_LEAP_YAER(p->ui_Year)) && (p->uc_Day == 29))
		return 1;
	
	// Check regular day 
	if(p->uc_Day> uc_ArrDaysInMonth[p->uc_Month])
		return 0;
	
	return 1;
}


////////////////////////////////////////////////////////////////////////////////////
//
// Api's
//
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//
// Function: i_GetMaxDateInMonth  
// Description: returns max days in month
// Parameters:
// Return: errur return -1, other - true
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

int i_GetMaxDateInMonth(st_Time tm)
{
	// Check special day 29 of febuary in a leap year 
	if((tm.uc_Month == FEBRUARY) && (IS_LEAP_YAER(tm.ui_Year))) 
		return 29;
	
	//return regular day 
	return uc_ArrDaysInMonth[tm.uc_Month];
	
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: l_GetSecFrom2000Now  
// Description: 
// Parameters:
// Return: 
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

long l_GetSecFrom2000Now(void)
{
	return l_TimeH_ConvertStTime2SecFrom2000(NULL);
}
////////////////////////////////////////////////////////////////////////////////////
//
// Function: l_ConvertStTime2SecFrom2000 
// Description: Converts time struct to seconds from 20000
// Parameters:
// Return: Sec From 2000. if errur return -1
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

long l_TimeH_ConvertStTime2SecFrom2000 ( const st_Time *stp_TimeIn)   // [IN] time struct needs to be converted	
{
	
	long l_Result;
	int b_FlagFebruar;
	unsigned int year,month;
	
	b_FlagFebruar = 0;
	l_Result=0;
	year=month=0;
	
	
	// If request for current time
	if(stp_TimeIn==0)
	{
		st_Time st_Now;
		
		
		l_Result = l_TimeH_ConvertStTime2SecFrom2000 (&st_Now);
		return l_Result;
	}
	
	// Start calculate seconds
	// Validate input 
	
	if( !b_IsInputTimeStValid(stp_TimeIn)) 
		return -1; // error
	
	// Years
	for (year = 2000; year < stp_TimeIn->ui_Year ; year++)
		for (month = 1; month <= 12; month++)
		{
			l_Result += (long)(uc_ArrDaysInMonth[month])*(long)SECONDS_IN_DAY;
			if (month == FEBRUARY && IS_LEAP_YAER(year))
				l_Result += SECONDS_IN_DAY; // add one day
		}
		
		// Month
		b_FlagFebruar = IS_LEAP_YAER(stp_TimeIn->ui_Year);
		for (month = 1; month < stp_TimeIn->uc_Month; month++)
		{
			l_Result += (long)(uc_ArrDaysInMonth[month])*(long)SECONDS_IN_DAY;
			if (month == FEBRUARY && b_FlagFebruar)
				l_Result += SECONDS_IN_DAY;
		}
		// Day + Hour + tm_min + sec
		l_Result += ( ((long)(stp_TimeIn->uc_Day -1 )*(long)SECONDS_IN_DAY)+    // DAY
			((long)(stp_TimeIn->uc_Hour) * (long)SECONDS_IN_HOUR) +             // HOUR
			((long)stp_TimeIn->uc_Minute * (long)SECONDS_IN_MINUTE) +           // MINUTE
			((long)stp_TimeIn->uc_Second)  );                                   // Second
		
		return l_Result;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: b_Convert2000Sec2StTime 
// Description:Convert secondes from 2000 input to Struct Time 
// Parameters:
// Return:true - success, false - error 
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

int b_TimeH_Convert2000Sec2StTime (
								   const unsigned long l_SecIn,            // [IN]  secondes since 2000
								   st_Time *stp_TimeOut                    // [OUT] time struct needs to be filled
								   )
{
	// Validate input
	long l_RemainSec,l_SecInMonth,l_SecInYear;
	if (!stp_TimeOut)
		return 0;
	
	// Start Value
	
	stp_TimeOut->ui_Year    = 2000;
	stp_TimeOut->uc_Month   = 1;
	stp_TimeOut->uc_Day     = 1;
	stp_TimeOut->uc_Hour    = 0;
	stp_TimeOut->uc_Minute  = 0;
	stp_TimeOut->uc_Second  = 0;
	
	if(l_SecIn == 0 )
		return 1;
	
	// Years
	l_RemainSec = l_SecIn;
	
	l_SecInYear = GET_NUM_OF_SEC_IN_YEAR(stp_TimeOut->ui_Year);
	while(l_RemainSec >= l_SecInYear  )
	{
		stp_TimeOut->ui_Year++;
		l_RemainSec-=l_SecInYear;
		
		// Calc next year
		l_SecInYear = GET_NUM_OF_SEC_IN_YEAR(stp_TimeOut->ui_Year);
	}
	if(l_RemainSec<=0)
		return 1;
	
	// Monthes
	l_SecInMonth = GET_NUM_OF_SEC_IN_MONTH(stp_TimeOut->ui_Year,stp_TimeOut->uc_Month);
	while(l_RemainSec >= l_SecInMonth  )
	{
		stp_TimeOut->uc_Month++;
		l_RemainSec-=l_SecInMonth;
		
		// Calc next month
		l_SecInMonth = GET_NUM_OF_SEC_IN_MONTH(stp_TimeOut->ui_Year,stp_TimeOut->uc_Month);
	}
	if(l_RemainSec<=0)
		return 1;
	
	// Days
	while(l_RemainSec >= SECONDS_IN_DAY  )
	{
		stp_TimeOut->uc_Day++;
		l_RemainSec-=SECONDS_IN_DAY;
	}
	if(l_RemainSec<=0)
		return 1;
	
	
	// Hour
	while(l_RemainSec >= SECONDS_IN_HOUR  )
	{
		stp_TimeOut->uc_Hour++;
		l_RemainSec-=SECONDS_IN_HOUR;
	}
	if(l_RemainSec<=0)
		return 1;
	
	// Minutes
	while(l_RemainSec >= SECONDS_IN_MINUTE  )
	{
		stp_TimeOut->uc_Minute++;
		l_RemainSec-=SECONDS_IN_MINUTE;
	}
	if(l_RemainSec<=0)
		return 1;
	
	// Seconds
	stp_TimeOut->uc_Second = (unsigned char)l_RemainSec;
	return 1;
	
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: v_TimeH_Time2String 
// Description: Convert the   time struct  into string
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

void v_TimeH_Time2String (
						  const st_Time *stp_TimeIn,  // [IN] time struct needs to be converted
						  char *cp_StringOut          //[OUT] StringOut: the converted result : dd/mm/yyyy  hh/mm/ss
						  )
{
	
	// 10/03/2003 23:59
	if( cp_StringOut&&stp_TimeIn)
	{
		// Micky 12/10/10
		sprintf(cp_StringOut,"%.2d/%.2d/%.2d %.2d:%.2d",
			((int)stp_TimeIn->uc_Day) % 100, ((int)stp_TimeIn->uc_Month) % 100, stp_TimeIn->ui_Year % 100, 
			((int)stp_TimeIn->uc_Hour) % 100,((int)stp_TimeIn->uc_Minute) % 100);
	}
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function:  v_ConvertSt_Time2YYYYMMDDhhmm
// Description: converts st_Time to YYYYMMDDhhmm format
// Parameters:
// Return:none
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

void v_ConvertSt_Time2YYYYMMDDhhmm(st_Time* tm, // [IN]time and date in st_Time format
								   char* OutTime,                              // [OUT] time and date string
								   int len)                                    // [IN]size of OutTime Array
								   
{
	if (len < 13)
		return;
	
	sprintf(OutTime, "%d%.2d%.2d%.2d%.2d",
		tm->ui_Year % 10000, ((int)tm->uc_Month) % 100, ((int)tm->uc_Day) % 100,
		((int)tm->uc_Hour) % 100, ((int)tm->uc_Minute) % 100);
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: ov_Str2DateTime 
// Description: converts time string to st_Time structure
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

st_Time ov_Str2DateTime (char* cp_DateStr, e_ModeDate e_Mode)
{
	st_Time ov_TimeDate;
	char cv_TempStr[20];
	
	memset(&ov_TimeDate, 0, sizeof(st_Time));
	
	// Copy year
	strcpy (cv_TempStr, cp_DateStr);
	if ((e_Mode == e_DocsMode) || (e_Mode == e_DocsMode2)) // YYYYMMDDhhmm or YYYYMMDDhhmmss
	{
		cv_TempStr[4]=0;
		ov_TimeDate.ui_Year = atoi (&cv_TempStr[0]);
		
		// Copy month
		strcpy(cv_TempStr, cp_DateStr);
		cv_TempStr[6]=0;
		ov_TimeDate.uc_Month = atoi (&cv_TempStr[4]);
		
		// Copy date
		strcpy (cv_TempStr, cp_DateStr);
		cv_TempStr[8]=0;
		ov_TimeDate.uc_Day = atoi (&cv_TempStr[6]);
		
		// Copy hour
		strcpy (cv_TempStr, cp_DateStr);
		cv_TempStr[10]=0;
		ov_TimeDate.uc_Hour = atoi (&cv_TempStr[8]);
		
		//  Copy minutes
		strcpy (cv_TempStr, cp_DateStr);
		cv_TempStr[12]=0;
		ov_TimeDate.uc_Minute = atoi (&cv_TempStr[10]);
		if (e_Mode == e_DocsMode2) // YYYYMMDDhhmmss
		{
			// Copy minutes
			strcpy (cv_TempStr, cp_DateStr);
			cv_TempStr[14] = 0;
			ov_TimeDate.uc_Second = atoi(&cv_TempStr[12]);
		}
		else
			ov_TimeDate.uc_Second = 0;
	}
	else if (e_Mode == e_WaitMode) //dd:mm:yyyy:hh:mm
	{
		cv_TempStr[2]=0;
		ov_TimeDate.uc_Day = atoi (cv_TempStr);
		cv_TempStr[5]=0;
		ov_TimeDate.uc_Month = atoi (&cv_TempStr[3]);
		cv_TempStr[10]=0;
		ov_TimeDate.ui_Year = atoi (&cv_TempStr[6]);
		cv_TempStr[13]=0;
		ov_TimeDate.uc_Hour = atoi (&cv_TempStr[11]);
		ov_TimeDate.uc_Minute = atoi (&cv_TempStr[14]);
		ov_TimeDate.uc_Second = 0;
	} //end switch
	
	
	return ov_TimeDate; //  return result structure
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function:  
// Description: 
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

// Get gmt offset
// 0: No data in static tables for the input date
// else: GNT offset (2 or 3)

int i_TimeH_GetIsraelGMTOffset(st_Time *Now)
{
	
	int i = 0;
	while(IsrDSTTable[i].ui_Year > 0)
	{
		if(Now->ui_Year == IsrDSTTable[i].ui_Year)
		{
			
			if((Now->uc_Month >= IsrDSTTable[i].uc_StartMonth) &&  (Now->uc_Month <= IsrDSTTable[i].uc_EndMonth))
			{
				
				
				// Make sure we are within the days range (start of summer clock)
				if((Now->uc_Month == IsrDSTTable[i].uc_StartMonth) && (Now->uc_Day < IsrDSTTable[i].uc_StartDay))
					return 2;
				
				// Make sure we are within the days range (end summer clock)
				if((Now->uc_Month == IsrDSTTable[i].uc_EndMonth) && (Now->uc_Day > IsrDSTTable[i].uc_EndDay))
					return 2;
				
				
				// Make sure we are within the hours range (start of summer clock)
				if((Now->uc_Month == IsrDSTTable[i].uc_StartMonth) && (Now->uc_Day == IsrDSTTable[i].uc_StartDay))
				{
					if(Now->uc_Hour < IsrDSTTable[i].uc_StartHour)
						return 2; // Winter clock
				}
				
				// Make sure we are within the hours range (end summer clock)
				if(((Now->uc_Month == IsrDSTTable[i].uc_EndMonth) && Now->uc_Day == IsrDSTTable[i].uc_EndDay))
				{
					if(Now->uc_Hour > IsrDSTTable[i].uc_EndHour)
						return 2; // Winter clock
				}
				
				return 3;
			}
			
			return 2;
		}
		
		
		i++;
	}
	
	return 0; // No data in table
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: i_TimeHGetNow 
// Description: Get current date and time
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

int i_TimeHGetNow (st_Time *Now)
{
#ifdef win_or_linux
    
    struct tm *newtime;
	time_t long_time;
	// Get PC time as long integer. 
	time( &long_time );               
	// Convert to local time. 
	newtime = localtime( &long_time );
    //convert to struct st_Time
	Now->ui_Year	=   newtime->tm_year+1900;	    /* Year	: for example 20000  */
	Now->uc_Month	=	newtime->tm_mon+1;		    /* Month.	[1-12] */
	Now->uc_Day     =   newtime->tm_mday;			/* Day.		[1-31] */
	Now->uc_Hour    =   newtime->tm_hour;			/* Hours.	[0-23] */
	Now->uc_Minute  =   newtime->tm_min;			/* Minutes.	[0-59] */
	Now->uc_Second  =   newtime->tm_sec;			/* Seconds.	[0-59] */
	return 1;
	
#else 
	
	struct tm newtime;
	if(os_gettime(&newtime))
	{
		
        //convert to struct st_Time
		Now->ui_Year	=   newtime.tm_year;
		Now->uc_Month	=   newtime.tm_mon;	
		Now->uc_Day     =   newtime.tm_mday;		
		Now->uc_Hour    =   newtime.tm_hour;
		Now->uc_Minute  =   newtime.tm_min;	
		Now->uc_Second  =   newtime.tm_sec;	
		return 1;
		
	}
	
	return 0;
#endif        
	
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: i_TimeSet 
// Description: Set current date and time
// Parameters:
// Return:
// Logic:
//
////////////////////////////////////////////////////////////////////////////////////

int i_TimeSet (st_Time *Now)
{
#ifdef win_or_linux
    
    
	return 1; // Nothing to di under Win32
	
#else 
	
	struct tm newtime;
	
	newtime.tm_year = Now->ui_Year;  
	newtime.tm_mon  = Now->uc_Month;  
	newtime.tm_mday = Now->uc_Day;  	
	newtime.tm_hour = Now->uc_Hour;  
	newtime.tm_min  = Now->uc_Minute;  
	newtime.tm_sec  = Now->uc_Second;  
	
	os_settime(&newtime);
	return 1;
#endif        
	
}

#endif // CORE_SUPPORT_LIBTIME
