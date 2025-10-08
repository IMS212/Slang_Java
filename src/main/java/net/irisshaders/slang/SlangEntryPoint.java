package net.irisshaders.slang;

import java.lang.foreign.MemorySegment;

public class SlangEntryPoint {
    protected final MemorySegment segment;
    private final String name;

    public SlangEntryPoint(String name, MemorySegment memorySegment) {
        this.name = name;
        this.segment = memorySegment;
    }

    public String getName() {
        return name;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        SlangEntryPoint that = (SlangEntryPoint) obj;
        return segment.equals(that.segment);
    }
}
