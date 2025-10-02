package net.irisshaders.slang;

import net.irisshaders.slang.internal.ListFilesFunction;
import net.irisshaders.slang.internal.LoadFileFunction;
import net.irisshaders.slang.internal.SlangJava;
import net.irisshaders.slang.internal.SlangString;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.util.function.BiConsumer;
import java.util.function.Function;

public class FileFinder {
    protected final MemorySegment segment;
    protected final MemorySegment segment2;
    private final Function<String, byte[]> loader;
    private final BiConsumer<String, FileFinderFunction> lister;
    private final Arena arena;
    private final FileLister listerImpl;

    public FileFinder(Function<String, byte[]> loader, BiConsumer<String, FileFinderFunction> lister) {
        this.listerImpl = new FileLister();
        this.loader = loader;
        this.lister = lister;
        this.arena = Arena.ofShared();

        this.segment = LoadFileFunction.allocate((f) -> {
            byte[] data = this.loader.apply(f.getString(0));
            if (data == null) return null;
            MemorySegment slangString = SlangString.allocate(arena);
            slangString.set(ValueLayout.ADDRESS, 0, arena.allocateFrom(ValueLayout.OfByte.JAVA_BYTE, data));
            slangString.set(ValueLayout.JAVA_INT, ValueLayout.ADDRESS.byteSize(), data == null ? 0 : data.length);
            return slangString;
        }, arena);

        this.segment2 = ListFilesFunction.allocate((path, callback, userData) -> {
            listerImpl.setCallback(callback, userData);
            lister.accept(path.getString(0), listerImpl::onFileFound);
            return 0;
        }, arena);
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
