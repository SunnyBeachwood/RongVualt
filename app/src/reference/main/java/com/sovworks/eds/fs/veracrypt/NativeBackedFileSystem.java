package com.sovworks.eds.fs.veracrypt;

import android.os.ParcelFileDescriptor;

import com.sovworks.eds.fs.Directory;
import com.sovworks.eds.fs.FSRecord;
import com.sovworks.eds.fs.File;
import com.sovworks.eds.fs.FileSystem;
import com.sovworks.eds.fs.Path;
import com.sovworks.eds.fs.RandomAccessIO;
import com.sovworks.eds.fs.errors.DirectoryIsNotEmptyException;
import com.sovworks.eds.fs.errors.FileSystemClosedException;
import com.sovworks.eds.fs.util.PathBase;
import com.sovworks.eds.fs.util.RandomAccessInputStream;
import com.sovworks.eds.fs.util.RandomAccessOutputStream;
import com.sovworks.eds.fs.util.Util;

import org.eds.veracrypt.domain.VolumeError;
import org.eds.veracrypt.nativecore.NativeFileEntry;
import org.eds.veracrypt.nativecore.NativeFileSystemAccess;
import org.eds.veracrypt.nativecore.NativeOpenFile;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

/**
 * Legacy EDS filesystem facade over a mounted native VeraCrypt session. It
 * never converts an encrypted volume path into a host filesystem path.
 * Closing this facade detaches the old UI only; the application session owner
 * remains responsible for locking the volume.
 */
public final class NativeBackedFileSystem implements FileSystem {
    private final NativeFileSystemAccess access;
    private boolean closed;

    public NativeBackedFileSystem(NativeFileSystemAccess access) {
        if (access == null) throw new IllegalArgumentException("Native filesystem access is required");
        this.access = access;
    }

    @Override public Path getRootPath() throws IOException { return getPath("/"); }
    @Override public Path getPath(String pathString) throws IOException { checkOpen(); return new NativePath(normalize(pathString)); }
    @Override public void close(boolean force) { closed = true; }
    @Override public boolean isClosed() { return closed; }

    private void checkOpen() throws FileSystemClosedException { if (closed) throw new FileSystemClosedException(); }
    private void requireWritable() throws IOException {
        checkOpen();
        if (access.isReadOnly()) throw new IOException("Native volume filesystem is read-only");
    }

    private static String normalize(String source) throws IOException {
        if (source == null || source.indexOf('\0') >= 0 || source.indexOf('\\') >= 0) throw new IOException("Invalid volume path");
        StringBuilder result = new StringBuilder();
        for (String part : source.split("/")) {
            if (part.isEmpty()) continue;
            if (part.equals(".") || part.equals("..")) throw new IOException("Invalid volume path");
            if (result.length() > 0) result.append('/');
            result.append(part);
        }
        return result.toString();
    }

    private NativeFileEntry stat(String path) throws IOException {
        if (path.isEmpty()) return null;
        try { return access.stat(path); }
        catch (Throwable error) {
            if (error instanceof VolumeError.NotFound) return null;
            throw io(error);
        }
    }

    private static IOException io(Throwable error) {
        return error instanceof IOException ? (IOException) error : new IOException("Native volume filesystem operation failed", error);
    }

    private final class NativePath extends PathBase {
        private final String path;
        NativePath(String path) { super(NativeBackedFileSystem.this); this.path = path; }
        @Override public String getPathString() { return path.isEmpty() ? "/" : "/" + path; }
        @Override public boolean exists() throws IOException { checkOpen(); return path.isEmpty() || stat(path) != null; }
        @Override public boolean isFile() throws IOException { NativeFileEntry entry = stat(path); return entry != null && !entry.isDirectory(); }
        @Override public boolean isDirectory() throws IOException { return path.isEmpty() || (stat(path) != null && stat(path).isDirectory()); }
        @Override public Directory getDirectory() throws IOException { if (!isDirectory() && exists()) throw new IOException("Path is not a directory"); return new NativeDirectory(this); }
        @Override public File getFile() throws IOException { if (exists() && !isFile()) throw new IOException("Path is not a file"); return new NativeFile(this); }
    }

    private abstract class NativeRecord implements FSRecord {
        NativePath path;
        NativeRecord(NativePath path) { this.path = path; }
        @Override public Path getPath() { return path; }
        @Override public String getName() { return path.getPathUtil().getFileName(); }
        @Override public Date getLastModified() throws IOException { NativeFileEntry entry = stat(path.path); return entry == null ? new Date(0) : new Date(fatTime(entry)); }
        @Override public void setLastModified(Date dt) throws IOException { throw new IOException("Setting timestamps is not supported by the native filesystem bridge"); }
        @Override public void delete() throws IOException { requireWritable(); try { access.delete(path.path); } catch (Throwable error) { throw io(error); } }
        @Override public void rename(String name) throws IOException { move(path.getParentPath(), name); }
        @Override public void moveTo(Directory parent) throws IOException { move(parent.getPath(), getName()); }
        private void move(Path parent, String name) throws IOException {
            requireWritable();
            if (!(parent instanceof NativePath)) throw new IOException("Destination belongs to another filesystem");
            String target = normalize(parent.getPathString() + "/" + name);
            try { access.rename(path.path, target); path = new NativePath(target); } catch (Throwable error) { throw io(error); }
        }
    }

    private final class NativeDirectory extends NativeRecord implements Directory {
        NativeDirectory(NativePath path) { super(path); }
        @Override public Directory createDirectory(String name) throws IOException { requireWritable(); String child = normalize(path.getPathString() + "/" + name); try { access.createDirectory(child); return new NativeDirectory(new NativePath(child)); } catch (Throwable error) { throw io(error); } }
        @Override public File createFile(String name) throws IOException { requireWritable(); String child = normalize(path.getPathString() + "/" + name); try (NativeOpenFile ignored = access.openFile(child, true, true, false)) { return new NativeFile(new NativePath(child)); } catch (Throwable error) { throw io(error); } }
        @Override public Contents list() throws IOException {
            try {
                List<Path> paths = new ArrayList<>();
                for (NativeFileEntry entry : access.list(path.path)) paths.add(new NativePath(normalize(path.getPathString() + "/" + entry.getName())));
                return new Contents() { @Override public void close() {} @Override public Iterator<Path> iterator() { return paths.iterator(); } };
            } catch (Throwable error) { throw io(error); }
        }
        @Override public void delete() throws IOException {
            try { if (!access.list(path.path).isEmpty()) throw new DirectoryIsNotEmptyException(path.getPathString()); super.delete(); }
            catch (DirectoryIsNotEmptyException error) { throw error; } catch (Throwable error) { throw io(error); }
        }
        @Override public long getTotalSpace() throws IOException { throw new IOException("Filesystem capacity is not exposed by the native bridge"); }
        @Override public long getFreeSpace() throws IOException { throw new IOException("Filesystem free space is not exposed by the native bridge"); }
    }

    private final class NativeFile extends NativeRecord implements File {
        NativeFile(NativePath path) { super(path); }
        @Override public InputStream getInputStream() throws IOException { return new RandomAccessInputStream(getRandomAccessIO(AccessMode.Read)); }
        @Override public OutputStream getOutputStream() throws IOException { return new RandomAccessOutputStream(getRandomAccessIO(AccessMode.Write)); }
        @Override public RandomAccessIO getRandomAccessIO(AccessMode mode) throws IOException {
            boolean writable = mode != AccessMode.Read;
            if (writable) requireWritable();
            try {
                NativeOpenFile file = access.openFile(path.path, writable, writable, mode == AccessMode.Write || mode == AccessMode.ReadWriteTruncate);
                NativeRandomAccessIO io = new NativeRandomAccessIO(file, writable, path);
                if (mode == AccessMode.WriteAppend) io.seek(getSize());
                return io;
            } catch (Throwable error) { throw io(error); }
        }
        @Override public long getSize() throws IOException { NativeFileEntry entry = stat(path.path); if (entry == null) throw new IOException("File does not exist"); return entry.getSizeBytes(); }
        @Override public ParcelFileDescriptor getFileDescriptor(AccessMode mode) throws IOException { throw new IOException("Use DocumentsProvider proxy descriptors for native volumes"); }
        @Override public void copyToOutputStream(OutputStream output, long offset, long count, ProgressInfo progress) throws IOException { Util.copyFileToOutputStream(output, this, offset, count, progress); }
        @Override public void copyFromInputStream(InputStream input, long offset, long count, ProgressInfo progress) throws IOException { Util.copyFileFromInputStream(input, this, offset, count, progress); }
    }

    private final class NativeRandomAccessIO implements RandomAccessIO {
        private final NativeOpenFile file; private final boolean writable; private final NativePath path; private long position;
        NativeRandomAccessIO(NativeOpenFile file, boolean writable, NativePath path) { this.file = file; this.writable = writable; this.path = path; }
        @Override public void seek(long value) throws IOException { if (value < 0) throw new IOException("Negative seek"); position = value; }
        @Override public long getFilePointer() { return position; }
        @Override public long length() throws IOException { NativeFileEntry entry = stat(path.path); if (entry == null) throw new IOException("File does not exist"); return entry.getSizeBytes(); }
        @Override public int read() throws IOException { byte[] one = new byte[1]; int read = read(one, 0, 1); return read < 0 ? -1 : one[0] & 0xff; }
        @Override public int read(byte[] data, int offset, int length) throws IOException { try { int read = file.read(position, data, offset, length); if (read > 0) position += read; return read == 0 && length > 0 ? -1 : read; } catch (Throwable error) { throw io(error); } }
        @Override public void write(int value) throws IOException { write(new byte[] {(byte) value}, 0, 1); }
        @Override public void write(byte[] data, int offset, int length) throws IOException { if (!writable) throw new IOException("File is read-only"); try { int written = file.write(position, data, offset, length); if (written != length) throw new IOException("Short write"); position += written; } catch (Throwable error) { throw io(error); } }
        @Override public void flush() throws IOException { try { file.flush(); } catch (Throwable error) { throw io(error); } }
        @Override public void setLength(long length) throws IOException { if (!writable) throw new IOException("File is read-only"); try { file.truncate(length); if (position > length) position = length; } catch (Throwable error) { throw io(error); } }
        @Override public void close() throws IOException { try { if (writable) file.flush(); file.close(); } catch (Throwable error) { throw io(error); } }
    }

    private static long fatTime(NativeFileEntry entry) {
        int date = entry.getModifiedDate(), time = entry.getModifiedTime();
        java.util.Calendar value = java.util.Calendar.getInstance();
        value.clear(); value.set(1980 + ((date >>> 9) & 0x7f), ((date >>> 5) & 0x0f) - 1, date & 0x1f, (time >>> 11) & 0x1f, (time >>> 5) & 0x3f, (time & 0x1f) * 2);
        return value.getTimeInMillis();
    }
}
