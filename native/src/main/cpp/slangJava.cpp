#include "slangJava.h"

#include <slang-com-helper.h>
#include <slang-com-ptr.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "slang.h"

#include <cstring>
#include <format>

void diagnoseIfNeeded(slang::IBlob *diagnosticsBlob) {
    if (diagnosticsBlob != nullptr) {
        std::cout << (const char *) diagnosticsBlob->getBufferPointer() << std::endl;
    }
}

struct SlangGlobalSessionWrapper {
    Slang::ComPtr<slang::IGlobalSession> session;
};

struct SlangSessionWrapper {
    Slang::ComPtr<slang::ISession> session;
    Slang::ComPtr<ISlangFileSystem> fileSystem; // keep a ref in case Slang doesn't
};

struct SlangModuleWrapper {
    Slang::ComPtr<slang::IModule> module;
};

struct SlangEntryPointWrapper {
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
};

struct SlangProgramWrapper {
    Slang::ComPtr<slang::IComponentType> program;
};

struct SlangLinkedWrapper {
    Slang::ComPtr<slang::IComponentType> program;
};

class ApBlob final : public ISlangBlob {
    public:
        char *data;
        int size;

        ApBlob(char *data, int size) {
            this->data = data;
            this->size = size;
        }

        virtual ~ApBlob() = default;

        SlangResult queryInterface(const SlangUUID &uuid, void **outObject) {
            return SLANG_E_NO_INTERFACE;
        }

        uint32_t addRef() {
            return 1;
        }

        uint32_t release() {
            return 0;
        }

        const void *getBufferPointer() {
            return data;
        }

        size_t getBufferSize() {
            return size;
        }
};

class ApFileSystem final : public ISlangFileSystemExt {
    public:
        LoadFileFunction funct;
        ListFilesFunction lister;
        CombinePathsFunction combiner;
        CheckIsDirectoryFunction checker;

        explicit ApFileSystem(LoadFileFunction funct, ListFilesFunction lister, CombinePathsFunction combiner,
                             CheckIsDirectoryFunction checker) {
            this->funct = funct;
            this->lister = lister;
            this->combiner = combiner;
            this->checker = checker;
        }

        virtual ~ApFileSystem() = default;

        SlangResult loadFile(const char *path, ISlangBlob **outBlob) override {
            SlangString *blob = funct(path);

            if (blob->size == 0 || !blob->data) {
                return SLANG_E_NOT_FOUND;
            }

            ISlangBlob *rawBlob = slang_createBlob(blob->data, blob->size);

            *outBlob = rawBlob;

            return SLANG_OK;
        }

        SlangResult queryInterface(const SlangUUID &uuid, void **outObject) override {
            if (!outObject) {
                return SLANG_E_INVALID_ARG;
            }

            *outObject = nullptr;

            if (uuid == ISlangFileSystem::getTypeGuid()) {
                *outObject = static_cast<ISlangFileSystem *>(this);
                addRef();
                return SLANG_OK;
            }

            return SLANG_E_NO_INTERFACE;
        }

        uint32_t addRef() override {
            return 1;
        }

        uint32_t release() override {
            return 0;
        }

        void *castAs(const SlangUUID &guid) override {
            if (guid == getTypeGuid()) {
                return static_cast<ISlangFileSystem *>(this);
            }
            return nullptr;
        }


        void clearCache() {
        }

        SlangResult enumeratePathContents(const char *path, FileSystemContentsCallBack callback, void *userData) {
            return lister(path, (void *) callback, userData);
        }

        OSPathKind getOSPathKind() override {
            return OSPathKind::Direct;
        }

        SlangResult getFileUniqueIdentity(const char *path, ISlangBlob **outUniqueIdentity) override {
            *outUniqueIdentity = slang_createBlob(path, strlen(path));
            return SLANG_OK; // TODO: this is... wrong. Paths should be resolved to absolute paths first, and then hashed.
        }

        SlangResult calcCombinedPath(SlangPathType fromPathType, const char *path1, const char *path2,
            ISlangBlob **pathOut) override {
            int size = -1;

            char* path = combiner(path1, path2, &size);
            if (size <= 0 || !path) {
                printf("Couldn't get combined path\n");
                return SLANG_E_NOT_FOUND;
            }

            *pathOut = slang_createBlob(path, size);
            return SLANG_OK;
        }

        SlangResult getPathType(const char *path, SlangPathType *pathTypeOut) override {
            bool isDir = checker(path) > 0;
            *pathTypeOut = isDir ? SLANG_PATH_TYPE_DIRECTORY : SLANG_PATH_TYPE_FILE;
            return SLANG_OK;
        }

        SlangResult getPath(PathKind kind, const char *path, ISlangBlob **outPath) override {
            printf("FUCK\n");
            *outPath = slang_createBlob(path, strlen(path));
            return SLANG_OK;
        }
};

char *nullTerminateThis(const char *src, std::size_t len) {
    char *out = new char[len + 1];
    std::memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

extern "C" {
    int fuck(int va) {
        return 69;
    }

    static void listResources(slang::ProgramLayout *prog, int targetIndex, const void **onError) {
        // Use the per-target layout
        auto *tgt = prog;
        auto *globalsVL = tgt->getGlobalParamsVarLayout();
        auto *globalsTL = tgt->getGlobalParamsTypeLayout();

        const int rangeCount = globalsTL->getBindingRangeCount();
        for (int r = 0; r < rangeCount; ++r) {
            const auto brType = globalsTL->getBindingRangeType(r); // slang::BindingType in your build
            const int brCount = globalsTL->getBindingRangeBindingCount(r); // array size (>=1)

            // Map to a GL-ish kind just for printing
            const char *glKind =
                    (brType == slang::BindingType::ConstantBuffer)
                        ? "UBO"
                        : (brType == slang::BindingType::MutableTexture)
                              ? "ImageUnit"
                              : (brType == slang::BindingType::Sampler ||
                                 brType == slang::BindingType::CombinedTextureSampler)
                                    ? "SamplerUnit"
                                    : (brType == slang::BindingType::MutableTypedBuffer ||
                                       brType == slang::BindingType::MutableRawBuffer)
                                          ? "SSBO"
                                          : "Texture/BufferUnit";

            slang::ParameterCategory cat;
            if (brType == slang::BindingType::ConstantBuffer) {
                cat = slang::ParameterCategory::ConstantBuffer;
            } else if (
                brType == slang::BindingType::Sampler ||
                brType == slang::BindingType::CombinedTextureSampler) {
                cat = slang::ParameterCategory::SamplerState;
            } else if (
                brType == slang::BindingType::Texture ||
                brType == slang::BindingType::MutableTexture) {
                cat = slang::ParameterCategory::UnorderedAccess;
            } else {
                cat = slang::ParameterCategory::DescriptorTableSlot;
            }

            // Space (descriptor set / register space) for this binding range
            const uint32_t space = globalsTL->getBindingRangeDescriptorSetIndex(r);

            // Translate binding-range -> descriptor-range -> API binding index
            const int drFirst = globalsTL->getBindingRangeFirstDescriptorRangeIndex(r);
            // Usually a single descriptor range; iterate if you want to print both parts
            // of a combined texture+sampler, etc.
            const uint32_t binding = globalsTL->getDescriptorSetDescriptorRangeIndexOffset(0, r);

            const char *name = nullptr;
            if (r < globalsTL->getFieldCount())
                if (auto *f = globalsTL->getFieldByIndex(r)) name = f->getName();

            std::printf("space %u, binding %u, type %d (%s), count %d, name %s\n",
                        space, binding, int(brType), glKind, brCount, name ? name : "(unnamed)");
        }

        if (onError) *onError = new std::string("ok");
    }

    void ap_getVariables(SlangSessionWrapper *session, SlangProgramWrapper *program, ShaderVariable *data) {
        auto *tgt = program->program->getLayout(0);
        auto *globalsTL = tgt->getGlobalParamsTypeLayout();

        const int rangeCount = globalsTL->getBindingRangeCount();
        for (int r = 0; r < rangeCount; ++r) {
            const auto brType = globalsTL->getBindingRangeType(r);
            const int brCount = globalsTL->getBindingRangeBindingCount(r); // array size (>=1)


            slang::ParameterCategory cat;
            if (brType == slang::BindingType::ConstantBuffer) {
                cat = slang::ParameterCategory::ConstantBuffer;
            } else if (
                brType == slang::BindingType::Sampler ||
                brType == slang::BindingType::CombinedTextureSampler) {
                cat = slang::ParameterCategory::SamplerState;
            } else if (
                brType == slang::BindingType::Texture ||
                brType == slang::BindingType::MutableTexture) {
                cat = slang::ParameterCategory::UnorderedAccess;
            } else {
                cat = slang::ParameterCategory::DescriptorTableSlot;
            }

            const uint32_t space = globalsTL->getBindingRangeDescriptorSetIndex(r);

            const int binding = globalsTL->getDescriptorSetDescriptorRangeIndexOffset(0, r);

            const char *name = nullptr;
            if (r < globalsTL->getFieldCount())
                if (auto *f = globalsTL->getFieldByIndex(r)) name = f->getName();

            char *newString = strdup(name);
            data[r] = {
                .name = newString,
                .binding = binding,
                .set = 0,
                .type = static_cast<int>(cat)
            };
        }
    }


    int ap_getVariableCount(SlangSessionWrapper *session, SlangProgramWrapper *program, const void **onError) {
        // Use the per-target layout
        auto *tgt = program->program->getLayout(0);
        auto *globalsVL = tgt->getGlobalParamsVarLayout();
        auto *globalsTL = tgt->getGlobalParamsTypeLayout();

        slang::IModule *da;
        return globalsTL->getBindingRangeCount();
    }

    int ap_getModuleCount(SlangSessionWrapper *session) {
        return session->session->getLoadedModuleCount();
    }

    void ap_getModules(SlangSessionWrapper *session, ShaderModule *data) {
        int count = session->session->getLoadedModuleCount();
        for (int i = 0; i < count; i++) {
            auto *module = session->session->getLoadedModule(i);

            ISlangBlob *blob = nullptr;

            module->serialize(&blob);


            if (!blob) throw std::runtime_error("Failed to serialize module");

            const size_t sz = blob->getBufferSize();
            const void *src = blob->getBufferPointer();
            void *dst = nullptr;
            if (sz > 0 && src) {
                dst = std::malloc(sz);
                if (!dst) {
                    blob->Release();
                    throw std::bad_alloc();
                }
                std::memcpy(dst, src, sz);
            }

            data[i] = {
                .name = module->getName(),
                .path = module->getFilePath(),
                .data = dst,
                .size = static_cast<int>(sz)
            };

            blob->Release();
        }
    }


    void ap_freeModules(int count, ShaderModule *data) {
        for (int i = 0; i < count; i++) {
            // Free the buffer we allocated in ap_getModules
            if (data[i].data) {
                std::free(const_cast<void *>(data[i].data));
                data[i].data = nullptr;
            }
        }
    }

    void testReflection(slang::IComponentType &com, const void **onError) {
        slang::ProgramLayout *layout = com.getLayout(0);
        listResources(layout, 0, onError);
    }

    const char *result() {
        return "Fuck";
    }

    extern "C" SlangGlobalSessionWrapper *ap_createGlobalSession() {
        SlangGlobalSessionDesc desc = {};
        slang::IGlobalSession *raw = nullptr;

        SlangResult res = ::slang::createGlobalSession(&desc, &raw);
        if (SLANG_FAILED(res) || !raw) {
            return nullptr;
        }

        auto *wrapper = new SlangGlobalSessionWrapper{};
        wrapper->session.attach(raw);
        return wrapper;
    }

    extern "C" void ap_destroyGlobalSession(SlangGlobalSessionWrapper *wrapper) {
        if (!wrapper)
            return;
        if (wrapper->session)
            wrapper->session->release();
        delete wrapper;
    }

    SlangSessionWrapper *ap_createSession(SlangGlobalSessionWrapper *globalSession, LoadFileFunction file,
                                          ListFilesFunction lister, CombinePathsFunction combiner, CheckIsDirectoryFunction checker) {
        slang::SessionDesc desc = {};
        desc.allowGLSLSyntax = true;
        auto *entry = static_cast<slang::CompilerOptionEntry *>(malloc(sizeof(slang::CompilerOptionEntry) * 4));
        entry[0].name = slang::CompilerOptionName::VulkanInvertY;
        entry[0].value = {
            .intValue0 = 0
        };
        entry[1].name = slang::CompilerOptionName::MinimumSlangOptimization;
        entry[1].value = {
            .intValue0 = 1
        };
        entry[2].name = slang::CompilerOptionName::UseUpToDateBinaryModule;
        entry[2].value = {
            .intValue0 = 0 // for now
        };
        desc.compilerOptionEntries = entry;
        desc.compilerOptionEntryCount = 3;
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_GLSL;
        targetDesc.profile = globalSession->session->findProfile("glsl_460");

        desc.targets = &targetDesc;
        desc.targetCount = 1;
        desc.fileSystem = new ApFileSystem(file, lister, combiner, checker);
        desc.searchPathCount = 1;
        const char *data = "/";
        desc.searchPaths = &data;
        slang::ISession *session = nullptr;
        globalSession->session->createSession(desc, &session);
        auto *wrapper = new SlangSessionWrapper{};
        wrapper->session.attach(session);
        return wrapper;
    }

    void ap_onLoadFile(int isFolder, char *name, void *c, void *userData) {
        bool isFold = isFolder > 0;
        auto callback = FileSystemContentsCallBack(c);
        callback(isFold ? SlangPathType::SLANG_PATH_TYPE_DIRECTORY : SlangPathType::SLANG_PATH_TYPE_FILE, name,
                 userData);
    }

    SlangModuleWrapper *ap_loadModule(SlangSessionWrapper *session, char *path, const void **onError) {
        slang::IBlob *diagnosticsBlob = nullptr;

        try {
            auto module = session->session->loadModule(path, &diagnosticsBlob);
            auto *wrapper = new SlangModuleWrapper{};
            diagnoseIfNeeded(diagnosticsBlob);
            wrapper->module.attach(module);
            if (!module) {
                *onError = nullTerminateThis(static_cast<const char *>(diagnosticsBlob->getBufferPointer()),
                                             diagnosticsBlob->getBufferSize());
                return nullptr;
            }

            return wrapper;
        } catch (...) {
            diagnoseIfNeeded(diagnosticsBlob);

            printf("FUCK");
            return nullptr;
        }
    }

    int ap_getEntryPointCount(SlangModuleWrapper* module) {
        return module->module->getDefinedEntryPointCount();
    }

    void ap_getEntryPoints(SlangModuleWrapper* module, EntryPoints* entries) {
        int count = module->module->getDefinedEntryPointCount();
        for (int i = 0; i < count; i++) {
            slang::IEntryPoint* ep = nullptr;
            module->module->getDefinedEntryPoint(i, &ep);
            entries[i] = {
                .name = ep->getFunctionReflection()->getName(),
                .index = i
            };
        }
    }

    SlangModuleWrapper *ap_loadModuleFromCode(SlangSessionWrapper *session, char *name, char *path, char *data,
                                              const void **onError) {
        slang::IBlob *diagnosticsBlob = nullptr;
        auto module = session->session->loadModuleFromSourceString(name, path, data, &diagnosticsBlob);
        auto *wrapper = new SlangModuleWrapper{};
        diagnoseIfNeeded(diagnosticsBlob);
        wrapper->module.attach(module);
        if (!module) {
            *onError = nullTerminateThis(static_cast<const char *>(diagnosticsBlob->getBufferPointer()),
                                         diagnosticsBlob->getBufferSize());
            return nullptr;
        }
        return wrapper;
    }

    SlangEntryPointWrapper *ap_findEntryPoint(SlangModuleWrapper *module, char *path) {
        auto *entryPoint = new SlangEntryPointWrapper{};
        module->module->findEntryPointByName(path, entryPoint->entryPoint.writeRef());
        if (!entryPoint->entryPoint)
            return nullptr;
        return entryPoint;
    }

    const void *linkTesting(SlangSessionWrapper *session, SlangModuleWrapper *module,
                            SlangEntryPointWrapper *entryPoint) {
        std::array<slang::IComponentType *, 2> componentTypes = {module->module, entryPoint->entryPoint};
        Slang::ComPtr<slang::IComponentType> composedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = session->session->createCompositeComponentType(
                componentTypes.data(), componentTypes.size(), composedProgram.writeRef(), diagnosticsBlob.writeRef());
        }

        // 6. Link
        Slang::ComPtr<slang::IComponentType> linkedProgram;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());
        }

        // 7. Get Target Kernel Code
        Slang::ComPtr<slang::IBlob> spirvCode;
        {
            Slang::ComPtr<slang::IBlob> diagnosticsBlob;
            SlangResult result =
                    linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
        }
        return spirvCode->getBufferPointer();
    }

    SlangProgramWrapper *ap_compileProgram(SlangSessionWrapper *session, SlangEntryPointWrapper *entryPoint,
                                           void **modules, int size, const void **onError) {
        std::vector<slang::IComponentType *> components = {};
        for (int i = 0; i < size; ++i) {
            auto *wrap = static_cast<SlangModuleWrapper *>(modules[i]);
            components.push_back(wrap->module.get());
        }
        components.push_back(entryPoint->entryPoint.get());

        slang::IBlob *diagnosticsBlob = nullptr;
        auto *program = new SlangProgramWrapper{};

        session->session->createCompositeComponentType(components.data(), components.size(),
                                                       program->program.writeRef(), &diagnosticsBlob);

        //*onError = strdup(static_cast<SlangModuleWrapper *>(modules[0])->module->getName());

        diagnoseIfNeeded(diagnosticsBlob);

        return program;
    }

    SlangLinkedWrapper* ap_linkProgram(SlangSessionWrapper *session, SlangProgramWrapper *program,
                               const void **onError) {
        slang::IBlob *diagnosticsBlob = nullptr;

        SlangLinkedWrapper* prog = new SlangLinkedWrapper{};

        program->program->link(prog->program.writeRef(), &diagnosticsBlob);
        diagnoseIfNeeded(diagnosticsBlob);
        if (!prog->program.get()) {
            const char *ptr = static_cast<const char *>(diagnosticsBlob->getBufferPointer());
            size_t len = diagnosticsBlob->getBufferSize();

            auto *v = new std::string(ptr, len); // remember to delete v;
            *onError = v->c_str();
            return nullptr;
        }
        Slang::ComPtr<slang::IBlob> outCode;

        prog->program->getEntryPointCode(0, 0, outCode.writeRef(), &diagnosticsBlob);
        diagnoseIfNeeded(diagnosticsBlob);

        if (!outCode.get()) {
            const char *ptr = static_cast<const char *>(diagnosticsBlob->getBufferPointer());
            size_t len = diagnosticsBlob->getBufferSize();

            auto *v = new std::string(ptr, len); // remember to delete v;
            *onError = v->c_str();
            return nullptr;
        }



        return prog;
    }

    void ap_freeSegment(ShaderVariable *data, int variableCount) {
        for (int i = 0; i < variableCount; ++i) {
            free(data[i].name);
        }
    }
}
