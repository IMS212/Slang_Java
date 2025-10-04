package net.irisshaders.slang;

import net.irisshaders.slang.internal.EntryPoints;
import net.irisshaders.slang.internal.SlangJava;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

public class SlangModule {
    final MemorySegment segment;
    private final String name;
    private final Arena theArena;

    protected SlangModule(String name, MemorySegment segment, Arena theArena) {
        this.name = name;
        this.segment = segment;
        this.theArena = theArena;
    }

    public SlangEntryPoint findEntryPoint(String gaming) {
        MemorySegment seg = SlangJava.ap_findEntryPoint(segment, theArena.allocateFrom(gaming));
        if (seg.address() == 0) {
            throw new IllegalStateException("Failed to find entry point '" + gaming + "' in module '" + name + "'");
        }
        return new SlangEntryPoint(gaming, seg);
    }

    public String[] getEntryPoints() {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment countSeg = arena.allocate(ValueLayout.JAVA_INT);
            SlangJava.ap_getEntryPoints(segment, countSeg);
            int count = countSeg.get(ValueLayout.JAVA_INT, 0);
            String[] result = new String[count];
            MemorySegment array = EntryPoints.allocateArray(count, arena);
            SlangJava.ap_getEntryPoints(segment, array);
            for (int i = 0; i < count; i++) {
                result[i] = EntryPoints.name(EntryPoints.asSlice(array, i)).getString(0);
            }
            return result;
        }
    }
}
