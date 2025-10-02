package net.irisshaders.slang;

import net.irisshaders.slang.internal.SlangJava;

import java.lang.foreign.MemorySegment;

public class Slang {
    private static Slang INSTANCE;
    protected final MemorySegment segment;

    private Slang() {
        segment = SlangJava.ap_createGlobalSession();
    }

    public static Slang getInstance() {
        if (INSTANCE == null) INSTANCE = new Slang();
        return INSTANCE;
    }
}
