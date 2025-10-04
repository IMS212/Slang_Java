package net.irisshaders.slang;

import net.irisshaders.slang.internal.ShaderModule;
import net.irisshaders.slang.internal.SlangJava;
import org.lwjgl.system.libc.LibCStdlib;

import java.lang.foreign.Arena;
import java.lang.foreign.MemorySegment;
import java.lang.foreign.ValueLayout;
import java.util.Objects;

public class SlangSession {
    protected final MemorySegment segment;
    protected final Arena arena;

    public SlangSession(Slang global, FileFinder fileSystem) {
        segment = SlangJava.ap_createSession(global.segment, fileSystem.retriever, fileSystem.lister, fileSystem.combiner, fileSystem.checker);
        this.arena = Arena.ofShared();
    }

    public SlangModule loadModule(String moduleName) {
        try {
            MemorySegment error = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);
            MemorySegment module = arena.allocateFrom(moduleName);
            Objects.requireNonNull(module);
            MemorySegment result = SlangJava.ap_loadModule(segment, module, error);
            if (result.address() == 0) {
                String err = error.get(ValueLayout.ADDRESS, 0).reinterpret(Long.MAX_VALUE).getString(0);
                LibCStdlib.nfree(error.get(ValueLayout.ADDRESS, 0).address());
                throw new IllegalStateException("Failed to load module '" + moduleName + "': " + err);
            }
            return new SlangModule(moduleName, result, arena);
        } catch (IllegalStateException e) {
            throw new RuntimeException(e);
        }
    }

    public SlangModule loadModuleFromSource(String moduleName, String pathName, String codeS) {
        try {
            MemorySegment error = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);
            MemorySegment module = arena.allocateFrom(moduleName);
            MemorySegment path = arena.allocateFrom(pathName);
            MemorySegment code = arena.allocateFrom(codeS);
            Objects.requireNonNull(module);
            MemorySegment result = SlangJava.ap_loadModuleFromCode(segment, module, path, code, error);
            if (result.address() == 0) {
                String err = error.get(ValueLayout.ADDRESS, 0).reinterpret(Long.MAX_VALUE).getString(0);
                LibCStdlib.nfree(error.get(ValueLayout.ADDRESS, 0).address());
                throw new IllegalStateException("Failed to load module '" + moduleName + "': " + err);
            }
            return new SlangModule(moduleName, result, arena);
        } catch (IllegalStateException e) {
            throw new RuntimeException(e);
        }
    }

    public SlangCompile compileModules(SlangEntryPoint entry, SlangModule... modules) {
        MemorySegment error = arena.allocateFrom(ValueLayout.ADDRESS, MemorySegment.NULL);

        MemorySegment values = arena.allocate(ValueLayout.ADDRESS, modules.length);
        for (int i = 0; i < modules.length; i++) {
            values.set(ValueLayout.ADDRESS, i * ValueLayout.ADDRESS.byteSize(), modules[i].segment);
        }

        MemorySegment seg = SlangJava.ap_compileProgram(segment, entry.segment, values, modules.length, error);
        if (seg.address() == 0) {
            String err = error.get(ValueLayout.ADDRESS, 0).reinterpret(Long.MAX_VALUE).getString(0);
            System.out.println(err);
        }
        return new SlangCompile(seg, arena);
    }

    public ModuleIR[] getModules() {
        int count = SlangJava.ap_getModuleCount(segment);

        ModuleIR[] modules = new ModuleIR[count];

        try (Arena arena = Arena.ofConfined()) {
            MemorySegment data = ShaderModule.allocateArray(count, arena);
            SlangJava.ap_getModules(segment, data);

            for (int i = 0; i < count; i++) {
                MemorySegment slice = ShaderModule.asSlice(data, i);

                String name = ShaderModule.name(slice).getString(0);
                String path = ShaderModule.path(slice).getString(0);
                MemorySegment p = ShaderModule.data(slice);
                int size = ShaderModule.size(slice);
                if (size <= 0) throw new IllegalStateException("WHAT" + name);
                MemorySegment bytesSeg = p.reinterpret(size);
                byte[] module = bytesSeg.toArray(ValueLayout.JAVA_BYTE);
                modules[i] = new ModuleIR(name, path, module);
                System.out.println("Got a module of " + modules[i] + " with a size of " + ShaderModule.size(slice));
            }

            SlangJava.ap_freeModules(count, data);
        }

        return modules;
    }

    public void destroy() {
        SlangJava.ap_destroySession(segment);
        arena.close();
    }
}
