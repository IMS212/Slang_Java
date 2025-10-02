package net.irisshaders.slang;

import java.lang.foreign.MemorySegment;

public class SlangEntryPoint {
    protected final MemorySegment segment;

    public SlangEntryPoint(String gaming, MemorySegment memorySegment) {
        this.segment = memorySegment;
    }
}
