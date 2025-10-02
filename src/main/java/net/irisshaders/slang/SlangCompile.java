package net.irisshaders.slang;

import net.irisshaders.slang.internal.ShaderVariable;
import net.irisshaders.slang.internal.SlangJava;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;

public class SlangCompile {
    private final MemorySegment segment;

    public SlangCompile(MemorySegment seg) {
        this.segment = seg;
    }

    public String getCode(SlangSession session) {
        Arena arena = Arena.ofAuto();
        try {
            MemorySegment error = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);
            MemorySegment result = SlangJava.ap_linkProgram(session.segment, segment, error);
            if (result.address() == 0) {
                String err = error.get(ValueLayout.ADDRESS, 0).reinterpret(Long.MAX_VALUE).getString(0);
                throw new IllegalStateException("Failed to load module : " + err);
            }
            return result.getString(0);
        } catch (IllegalStateException e) {
            throw new RuntimeException(e);
        }
    }

    public ReflectedVariable[] getReflection(SlangSession session) {
        try (Arena arena = Arena.ofConfined()) {
            MemorySegment error = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);

            int variableCount = SlangJava.ap_getVariableCount(session.segment, segment, error);

            ReflectedVariable[] variables = new ReflectedVariable[variableCount];

            MemorySegment array = ShaderVariable.allocateArray(variableCount, arena);
            SlangJava.ap_getVariables(session.segment, segment, array);
            for (int i = 0; i < variableCount; i++) {
                MemorySegment slice = ShaderVariable.asSlice(array, i);
                variables[i] = new ReflectedVariable(
                        ShaderVariable.name(slice).getString(0),
                        ShaderVariable.binding(slice),
                        ShaderVariable.set(slice),
                        VariableType.fromInt(ShaderVariable.type(slice))
                );
            }

            SlangJava.ap_freeSegment(array, variableCount);

            return variables;
        }
    }
}
