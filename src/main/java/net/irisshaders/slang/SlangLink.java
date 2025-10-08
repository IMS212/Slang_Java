package net.irisshaders.slang;

import net.irisshaders.slang.internal.SlangJava;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

public class SlangLink {
    private final MemorySegment segment;
    private final Arena arena;
    private final SlangEntryPoint[] entryPoints;

    public SlangLink(MemorySegment result, Arena arena, SlangEntryPoint[] entryPoints) {
        this.segment = result;
        this.arena = arena;
        this.entryPoints = entryPoints;
    }

    public String getCode(SlangEntryPoint entryPoint) {
        for (int i = 0; i < entryPoints.length; i++) {
            SlangEntryPoint point = entryPoints[i];
            if (point.equals(entryPoint)) {
                MemorySegment outSize = arena.allocateFrom(ValueLayout.JAVA_INT, -1);
                MemorySegment onError = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);
                MemorySegment codeSeg = SlangJava.ap_getEntryPointCode(segment, i, outSize, onError);
                if (codeSeg.address() == 0) {
                    String err = onError.get(ValueLayout.ADDRESS, 0).reinterpret(Long.MAX_VALUE).getString(0);
                    throw new IllegalStateException("Failed to get entry point code for '" + entryPoint.getName() + "': " + err);
                }
                return new String(codeSeg.reinterpret(outSize.get(ValueLayout.JAVA_INT, 0)).toArray(ValueLayout.JAVA_BYTE));
            }
        }

        throw new IllegalArgumentException("Entry point not found in linked program: " + entryPoint);
    }
}
