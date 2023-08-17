package com.example.jnidemo;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import com.example.jnidemo.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity implements IAntiDebugCallback {

    // Used to load the 'jnidemo' library on application startup.
    static {
        System.loadLibrary("jnidemo");
    }

    private ActivityMainBinding binding;

    String text = "我是JAVA";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
        init();
    }

    /**
     * A native method that is implemented by the 'jnidemo' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void  test1();

    public void test1(View view) {
        test1();
    }

    public native void  test2();
    public void test2(View view) {
        test2();
        binding.sampleText.setText(text);
    }

    public native int nativeAdd(int x,int y);
    public void test3(View view) {
        binding.sampleText.setText(nativeAdd(1,2)+"");
    }


    public void test4(View view) {
        binding.sampleText.setText(getSignInfo());
    }

    /**
     * 初始化，获取应用签名信息,改成动态注册了，这个是静态注册
     * @return
     */
    public static native boolean init();

    /**
     * 获取配置的SDK key信息
     * @return
     */
    public static native String getKey();

    public void test5(View view) {
        binding.sampleText.setText(getKey());
    }

    public native String nativeMethod_key(Context context);

    public void test6(View view) {
        binding.sampleText.setText(nativeMethod_key(this));
    }

    public native void setAntiBiBCallback(IAntiDebugCallback callback);

    public void test7(View view) {
        setAntiBiBCallback(this);
    }

    @Override
    public void beInjectedDebug(String s) {
        binding.sampleText.setText(s);
    }

    /**
     * 获取签名唯一字符串
     */
    public String getSignInfo() {
        try {
            PackageInfo packageInfo = getPackageManager().getPackageInfo(
                    getPackageName(), PackageManager.GET_SIGNATURES);
            Signature[] signs = packageInfo.signatures;
            Signature sign = signs[0];
            Log.e("----->",sign.toCharsString());
            return sign.toCharsString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public void callBack(int code){
        Toast.makeText(this, "native层回调  " + code, Toast.LENGTH_SHORT).show();
    }
    public void callBack(byte[] code){
        Toast.makeText(this, "native层回调  " + code, Toast.LENGTH_SHORT).show();
    }
    //
    public String callBack(String code,int code1){
        Toast.makeText(this, "native层回调  " + code +"  code1:"+code1, Toast.LENGTH_SHORT).show();
        return null;
    }
}