# The 7-Zip binding discovers callbacks and native entry points reflectively.
-keep class net.sf.sevenzipjbinding.** { *; }
# Commons Compress discovers Brotli through Class.forName().
-keep class org.brotli.dec.** { *; }
-keep class org.eds.zipxtract.core.** { *; }
