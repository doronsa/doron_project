package com.commit.bsp;

//import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

public class BSPUtils extends Activity {
     
	 //XXX
	 static LoginHandler	mHandler;
	 private static class LoginHandler extends Handler {

		
		private WeakReference<BSPUtils>	mTarget;

 
		LoginHandler(BSPUtils target) {
			mTarget = new WeakReference<BSPUtils>(target);

		}
 

		public void setTarget(BSPUtils target) {
			mTarget.clear();

			mTarget = new WeakReference<BSPUtils>(target);
		}

 
		@Override
		public void handleMessage(Message msg) {
			BSPUtils activity = mTarget.get();
			if(msg != null && msg.obj != null)
			    activity.update(msg.obj);

		}
	}

//////////////////////////////////////////////////////////////////////////////////////////////////
	 //XXX
	 public static TextView textStatstat;
	
	 
	 private native String[] getGPSData();
	 
 
	 
	 public native int  sendCMD(String[] cmd);
	 public native void SetMsg(int id , int len , long vals[]);
	 public native int  initJNI();
	 int res;
	 TextView textStat;
	 SDUtils sdUtils;
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		textStat = (TextView) findViewById(R.id.textView1);
		sdUtils = new SDUtils();
		getActionBar().setTitle("BSP-BIT (ver 3.0.0)"); 
		initJNI();
		textStatstat = textStat; 
		
		//XXXXX
		if(mHandler == null)
			mHandler = new LoginHandler(this);
		else
			mHandler.setTarget(this);
		//XXXXX
	}

	/////////////////////////////////////////////////////////////////////////////////

    private void update(Object obj) 
	{
		updateStatusStr( (String)obj );

	}
	public void updateStatusStr(String str )
	{
		textStat.setText(str);
			
	}

///////////////////////////////////////////////////////////////////////////
	
	
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
		
	public void updateStatus(int res )
	{
		textStat.setText("Status "+res);
		android.util.Log.w("UI ", "Status : " + res);
	
	}
	
	public void updateCMDlen(int res )
	{
		android.util.Log.w("UI ", "Len : " + res);
	
	}
	
	public void InitPrinter(View view)  
	{  	    
		try {
			
						
			Toast.makeText(this, " InitPrinter!", Toast.LENGTH_SHORT).show();
			res = sendCMD(new String[]{"PRINTER_INIT"});
			updateStatus(res);
		} catch (Exception e) {
			Toast.makeText(this, e.toString(), Toast.LENGTH_SHORT).show(); 
		}
		
			
	    
	}  
	public void PrinterTest(View view)  
	{  
		try 
		{
			Toast.makeText(this, " InitPrinter!", Toast.LENGTH_SHORT).show();
			res = sendCMD(new String[]{"PRINTER_INIT"});
			updateStatus(res);
		} 
		catch (Exception e) 
		{
			Toast.makeText(this, e.toString(), Toast.LENGTH_SHORT).show(); 
		}
		
	    Toast.makeText(this, " PrinterTest!", Toast.LENGTH_SHORT).show(); 
	    res = sendCMD(new String[]{"PRINTER_TEST"});
	    updateStatus(res);
	    
	}  
	public void ModemConn(View view)  
	{  
	    Toast.makeText(this, " ModemConn!", Toast.LENGTH_SHORT).show(); 
	    res = sendCMD(new String[]{"MODEM_CONNECT"});
	    updateStatus(res);
	    
	} 
	public void ModemDrop(View view)  
	{  
	    Toast.makeText(this, " ModemDrop!", Toast.LENGTH_SHORT).show();  
	    res = sendCMD(new String[]{"MODEM_DISCONNECT"});
	    updateStatus(res);
	} 
	
	public void ModemStat(View view)  
	{  
	    Toast.makeText(this, " ModemStat!", Toast.LENGTH_SHORT).show();  
	    res = sendCMD(new String[]{"MODEM_STATUS"});
	    updateStatus(res);
	}  
	
	public void PrintImage(View view)  
	{  
	    Toast.makeText(this, " PrintImage!", Toast.LENGTH_SHORT).show(); 
	    res = sendCMD(new String[]{"PRINT_IMAGE","/data/data/hello3.bmp"});
	    updateStatus(res);
	}  
	public void CutPaper(View view)  
	{  
	    Toast.makeText(this, " CutPaper!", Toast.LENGTH_SHORT).show();
	    res = sendCMD(new String[]{"PRINTER_CUT"});
	    updateStatus(res);
	    
	}  
	
	public void GetGpioVal(View view)  
	{  
	    Toast.makeText(this, " Get GPIO Value", Toast.LENGTH_SHORT).show(); 
	    res = sendCMD(new String[]{"GPIO_VALUE","140"});//set the GPIO number
	    updateStatus(res);
	}  
	
			
	public void rssi_test(View view)
	{
//		Toast.makeText(this, " Check RSSI", Toast.LENGTH_SHORT).show();
//		android.util.Log.w("UI ", "Check RSSI ");
		
	}
	
	public void k10_test(View view)
	{
//		Toast.makeText(this, " Check Comm!", Toast.LENGTH_SHORT).show();
//		SetMsg(0 , 2 , new long []{1,2});
//		android.util.Log.w("UI ", "SetMsg k10 test ");
	    Toast.makeText(this, " Test K10 Com", Toast.LENGTH_SHORT).show(); 
	    res = sendCMD(new String[]{"K10_TEST_COM","140"});//set the GPIO number
	    updateStatus(res);
	}
	
	public void Amb_control_test(View view)
	{
		Toast.makeText(this, " SetAmbient", Toast.LENGTH_SHORT).show();
		SetMsg(94 , 2 , new long []{140,0});
		android.util.Log.w("UI ", "SetMsg SetAmbient ");
	}
	
	public void led_control_test(View view)
	{
		Toast.makeText(this, " Key Board LED", Toast.LENGTH_SHORT).show();
		SetMsg(92 , 2 , new long []{1,2});
		android.util.Log.w("UI ", "SetMsg Key Board LED ");
	}
	
	public void lcd_test(View view)
	{
		Toast.makeText(this, " lcd test", Toast.LENGTH_SHORT).show();
		SetMsg(4 , 2 , new long []{1,2});
		android.util.Log.w("UI ", "SetMsg lcd test ");
	}
	
	public void clear_all_led(View view)
	{
		Toast.makeText(this, " clear_all_led", Toast.LENGTH_SHORT).show();
		SetMsg(103 , 2 , new long []{1,2});
		//SetMsg(60 , 2 , new long []{1,2});//for test
		android.util.Log.w("UI ", "SetMsg clear_all_led 103");
//		SetMsg(104 , 2 , new long []{1,2});
//		android.util.Log.w("UI ", "SetMsg clear_all_led 104");
	}
	public void Get_GPS_Data(View view)
	{
		String [] GPSdada = getGPSData();
		
		for(int i = 0 ; i < GPSdada.length; i++)
		{
			
			android.util.Log.w("UI GPS", GPSdada[i]);
		}
		
		
		
		
		Toast.makeText(this, " Get_GPS_Data", Toast.LENGTH_SHORT).show();
		//GPSdada.
		//res = sendCMD(new String[]{"GET_GPS_DATA","1"});
		
		//GPSdada = getGPSData();
		//getGPSData();
	}
	
	public void SendCommandGetAppVer(View view)  
	{  
	    Toast.makeText(this, " GetAppVer!", Toast.LENGTH_SHORT).show();
	    //res = sendCMD(new String[]{"CMD_GET_APP_VERSION"});
	    //res = sendCMD(new Stri/media/doronsa/457cd718-9daa-4ce7-ba1a-42d4d4842eb5/data/doron/workspace/project/BSP/jning[]{"CMD_KEYBOARDCOMMANDCONFIG"});
	    //SetMsg(15 , 2 , new long []{1,2});// ok 
	    SetMsg(106 , 2 , new long []{1,2});// bsp version
	    //SetMsg(0 , 2 , new long []{1,2});// ok
	    //SetMsg(1 , 2 , new long []{1,2});// ok 
	    //SetMsg(2 , 15 , new long []{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});// ok
	    //SetMsg(3 , 2 , new long []{1,2});// send ok 
	    //SetMsg(4 , 20 , new long []{1,2,3,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,1,2});// ok   
	    //SetMsg(5 , 5 , new long []{1,2,3,4,5});// ok     
	    //SetMsg(6 , 1 , new long []{10});// send ok 
	    //SetMsg(7 , 1 , new long []{1,2});// send ok 
	    //SetMsg(8 , 2 , new long []{1,2});// ok ;
	    //SetMsg(9 , 2 , new long []{1,2});// ok ;
	    //SetMsg(10 , 2 , new long []{1,2});// send ok 
	    //SetMsg(11 , 2 , new long []{1,2});// ok
	    //SetMsg(12 , 2 , new long []{1,2});// ok
	    //SetMsg(13 , 4 , new long []{1,2,3,4});// ok 
	    //SetMsg(14 , 2 , new long []{1,2});// send ok 
	    //SetMsg(16 , 2 , new long []{1,2});//not tested
	    //SetMsg(17 , 2 , new long []{1,2});// ok
	    //SetMsg(18 , 2 , new long []{1,2});// ok
	    //SetMsg(19 , 2 , new long []{1,2});// ok
	    //SetMsg(21 , 2 , new long []{1,2});// // send ok 
	    //SetMsg(22 , 2 , new long []{1,2});// ok
	    //SetMsg(23 , 6 , new long []{1,2,3,4,5,6});// ok    
	    //SetMsg(92 , 2 , new long []{20,0});// send ok 
	    //SetMsg(93 , 2 , new long []{147,1});// send ok 
	    //TR_InitReader
	    //SetMsg(60 , 2 , new long []{147,1});// send ok 
	  //DownLoad file FLESH.BIN in /sdcard/
	   // SetMsg(95 , 6 , new long []{1,2});
	     
	}  

	static String [] K10_TYPES = new String []{"NONE", "BYTE","SHORT","NONE","DWORD"};
	
	public static void GetMsg(int id , int status, int[] types, Object[] data )
	{
		
		Message msg = new Message();
		msg.arg1 = id;
		String tempstr;
				
		//android.util.Log.w("UI ", "Befor for loop : " + id + " : " + "len:" + data.length);
		for (int i = 0; i < data.length; i++) 
		{
			;//android.util.Log.w("UI ", "GetMsg : " + id + " : " + K10_TYPES[types[i]] + " : " + data[i]);
		}	
		switch (id) 
		{
		case 0://e_CmdK10_CheckComm
			msg.obj = "K10 Test: "+data[0];
		    mHandler.sendMessage(msg);
			break;
		case 236://e_BSPGPIO_1:
			msg.obj = "GPIO-" + data[0] + " Value : " + data[1];
		    mHandler.sendMessage(msg);
			break;
		case 245://e_BSP_GET_VERSION
			tempstr = data[0].toString() + data[1].toString() +data[2].toString();
			msg.obj = "BSP Version-" + tempstr;
		    mHandler.sendMessage(msg);
			break;
		case 31://e_CmdK10_KeyBoardEvent
			msg.obj = "Key Pressed: " + data[2];
		    mHandler.sendMessage(msg);
			//android.util.Log.w("UI ", "GetMsg : " + id + " : " + K10_TYPES[types[0]] + " : " + data[0]);
			//Toast.makeText(null, "Key:" + data[0], Toast.LENGTH_SHORT).show();
		    break;
		case 32:
		case 14:
			android.util.Log.w("UI ", "GetMsg PeriodicMonitorEvent / PeriodicMonitorPoll ");
			break;
		default:
			break;
		}
	}
	
	
	static {
        System.loadLibrary("bsp-jni");
        
    }

}
