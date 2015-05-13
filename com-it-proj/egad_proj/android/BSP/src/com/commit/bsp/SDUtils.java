package com.commit.bsp;

import java.io.File;
import java.io.OutputStream;
import java.sql.Date;
import java.text.SimpleDateFormat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
//import android.os.Environment;
import android.provider.ContactsContract.Directory;
//import android.widget.Toast;

public class SDUtils extends BroadcastReceiver {

	  private final static String PATH = "/mnt/udisk";
    	
    @Override
    public void onReceive(Context context, Intent intent) {
    	  try {
			
    		  String action = intent.getAction();
              Uri uri = intent.getData();

              //are we interested in the broadcast?
              if(uri == null )
                       return;

              String path = intent.getData().getPath();
              //account for file:// prefix
              if(!path.endsWith(PATH))
                       return;

              File dir = new File(PATH);	  
    		  
   	
    	 
          if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
    		 
    		  if(dir.exists() /*&& dir.listFiles().length > 0*/)
    		     android.util.Log.w("BSP ", "SD Mounted : " + path);
    		 
    		  SimpleDateFormat sdf = new SimpleDateFormat("yyMMdd_HHmmss");
    		  String currentDateandTime = sdf.format(new Date(System.currentTimeMillis()));
    		  
    		  
    		     File file = new File(dir, "BSP_"+currentDateandTime+".txt");
    		     file.createNewFile();
    		      
    	  }
          else if (action.equals(Intent.ACTION_MEDIA_REMOVED)) {
    	 
    		  if(dir.listFiles().length == 0)
    		    android.util.Log.w("BSP ", "SD UnMounted : " );
    	  }
    	 
    	  } catch (Exception e) {
    		  android.util.Log.w("BSP ", e.getMessage() );
  		}
       
    }
   

   
}