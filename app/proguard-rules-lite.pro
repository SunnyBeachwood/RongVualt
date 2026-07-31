-dontobfuscate
-optimizations !code/allocation/variable,!code/simplification/arithmetic,!code/simplification/cast,!field/*,!class/merging/*

-keep public class com.sovworks.eds.android.EdsApplication { public *; }

# We want to keep methods in Activity that could be used in the XML attribute onClick
-keepclassmembers class * extends android.app.Activity {
   public void *(android.view.View);
}

-keep class * implements android.os.Parcelable {
  public static final android.os.Parcelable$Creator *;
}

-keepnames class * implements java.io.Serializable

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private static final java.io.ObjectStreamField[] serialPersistentFields;
    !static !transient <fields>;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

-keep public class com.sovworks.eds.fs.FileSystem
-keep public class com.sovworks.eds.fs.Path
-keep public class com.sovworks.eds.fs.RandomAccessIO
-keep interface com.sovworks.eds.settings.Settings

-keep public class com.sovworks.eds.android.views.GestureImageView$NavigListener
-keep public class com.sovworks.eds.android.views.GestureImageView$OptimImageRequiredListener

-keepclassmembers class * implements java.io.Externalizable {
    public void readExternal(java.io.ObjectInput);
    public void writeExternal(java.io.ObjectOutput);
}

-keep class androidx.** { *; }

-keep class org.apache.** { *; }

# The embedded file manager's syscall bridge resolves these constructors and
# fields from native code with JNI.  R8 may otherwise rewrite their signatures
# even when name obfuscation is disabled, causing a native abort at startup.
-keep class me.zhanghai.android.files.provider.linux.syscall.** { *; }
-keep class me.zhanghai.android.files.provider.common.ByteString { *; }

# vc_core invokes callbacks and constructs result/error objects through JNI.
# Keep the complete bridge surface: R8 cannot see these native lookups and
# would otherwise remove methods such as NativeUnlockProgress.onProgress().
-keep class org.eds.veracrypt.nativecore.VcCore { *; }
-keep class org.eds.veracrypt.nativecore.VcCoreFailure { *; }
-keep class org.eds.veracrypt.nativecore.NativeFileEntry { *; }
-keep class org.eds.veracrypt.nativecore.NativeUnlockProgress { *; }
-keep class org.eds.veracrypt.nativecore.NativeCreateProgress { *; }

-dontwarn org.apache.**
-dontwarn javax.servlet.**

-dontwarn androidx.**
-dontnote androidx.**

-dontwarn java.awt.*

# Material Files transitively brings optional desktop-only integrations. They
# are not packaged or reachable on Android, but R8 still sees their optional
# type references while shrinking the embedded file manager.
-dontwarn java.rmi.UnmarshalException
-dontwarn javax.el.**
-dontwarn org.ietf.jgss.**
-dontwarn pl.droidsonroids.gif.GifDrawable
-dontwarn sun.security.x509.X509Key
