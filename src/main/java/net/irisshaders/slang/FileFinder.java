package net.irisshaders.slang;

import net.irisshaders.slang.internal.*;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

public abstract class FileFinder {
    protected final MemorySegment retriever;
    protected final MemorySegment lister;
    protected final MemorySegment combiner;
    protected final MemorySegment checker;
    private final Arena arena;
    private final FileLister listerImpl;

    public abstract void forAllInFolder(String path, FileFinderFunction callback);

    public abstract byte[] getFile(String path);

    public abstract boolean isFolder(String path);

    public abstract String combinePath(String base, String relative);

    public FileFinder() {
        this.listerImpl = new FileLister();
        this.arena = Arena.ofShared();

        this.retriever = LoadFileFunction.allocate((f) -> {
            byte[] data = getFile(f.getString(0));
            if (data == null) return null;
            MemorySegment slangString = SlangString.allocate(arena);
            slangString.set(ValueLayout.ADDRESS, 0, arena.allocateFrom(ValueLayout.OfByte.JAVA_BYTE, data));
            slangString.set(ValueLayout.JAVA_INT, ValueLayout.ADDRESS.byteSize(), data == null ? 0 : data.length);
            return slangString;
        }, arena);

        this.lister = ListFilesFunction.allocate((path, callback, userData) -> {
            listerImpl.setCallback(callback, userData);
            forAllInFolder(path.getString(0), listerImpl::onFileFound);
            return 0;
        }, arena);

        this.checker = CheckIsDirectoryFunction.allocate((path) -> isFolder(path.getString(0)) ? 1 : 0, arena);

        this.combiner = CombinePathsFunction.allocate(((path1, path2, outPathSize) -> {
            MemorySegment segment = arena.allocateFrom(combinePath(path1.getString(0), path2.getString(0)));
            outPathSize.set(ValueLayout.JAVA_INT, 0L, (int) segment.byteSize());
            return segment;
        }), arena);
    }

    private static class FileLister {
        private MemorySegment callback;
        private MemorySegment userData;

        public void setCallback(MemorySegment callback, MemorySegment userData) {
            this.callback = callback;
            this.userData = userData;
        }

        public void onFileFound(boolean b, String string) {
            try (Arena arena = Arena.ofConfined()) {
                MemorySegment name = arena.allocateFrom(string);
                SlangJava.ap_onLoadFile(b ? 1 : 0, name, callback, userData);
            }
        }
    }
}
