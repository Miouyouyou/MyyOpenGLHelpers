/*
	Copyright (c) 2016 Miouyouyou <Myy>

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files 
	(the "Software"), to deal in the Software without restriction, 
  including	without limitation the rights to use, copy, modify, merge, 
  publish, distribute, sublicense, and/or sell copies of the Software, 
  and to permit persons to whom the Software is furnished to do so, 
  subject to the following conditions:

	The above copyright notice and this permission notice shall be 
  included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

package com.miouyouyou.gametests;

import android.app.NativeActivity;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;

public class NativeInsanity extends NativeActivity {

  public static class Log {
    public static final String LOG_TAG = "Java_native-insanity";
    public static void print(final String message, Object... args) {
      android.util.Log.e(LOG_TAG, String.format(message, args));
    }
  }

  public void openWebsite(final byte[] url_as_bytes) {
    final String url_as_string = new String(url_as_bytes);
    try {
      final Uri url = Uri.parse(url_as_string);
      startActivity(new Intent(Intent.ACTION_VIEW, url));
      Log.print("Website opened !\n");
    }
    catch(Exception e) {
      Log.print("Trying to open %s triggered : \n%s\n",
                url_as_string, e.getMessage());
    }
  }

  /* This avoid destroying and recreating the activity on screenChange,
   * which helps avoiding EGL_BAD_DISPLAY issues when locking the
   * screen orientation */
  @Override
  public void onConfigurationChanged(Configuration newConfig)
  {
      super.onConfigurationChanged(newConfig);
  }

}
