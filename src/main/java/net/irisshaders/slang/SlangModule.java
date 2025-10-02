package net.irisshaders.slang;

import net.irisshaders.slang.internal.SlangJava;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;

public class SlangModule {
    final MemorySegment segment;
    private final String name;

    protected SlangModule(String name, MemorySegment segment) {
        this.name = name;
        this.segment = segment;
    }

    public SlangEntryPoint findEntryPoint(String gaming) {
        Arena arena = Arena.ofAuto();

        MemorySegment seg = SlangJava.ap_findEntryPoint(segment, arena.allocateFrom(gaming));
        if (seg.address() == 0) {
            throw new IllegalStateException("Failed to find entry point '" + gaming + "' in module '" + name + "'");
        }
        return new SlangEntryPoint(gaming, seg);
    }
}
