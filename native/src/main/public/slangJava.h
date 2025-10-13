#ifndef SLANG_JAVA_SLANGJAVA_H
#define SLANG_JAVA_SLANGJAVA_H


#ifndef EXPORT_CODE
    #if defined(_WIN32) || defined(_WIN64)
        #define EXPORT_CODE __declspec(dllexport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define EXPORT_CODE __attribute__((visibility("default")))
    #else
        #define EXPORT_CODE
    #endif
#endif
#ifdef __cplusplus
extern "C" {
#endif

    struct SlangString {
        void *data;
        int size;
    };

    typedef struct ShaderVariable {
        char *name;
        int binding;
        int set;
        int type;
    } ShaderVariable;

    typedef struct EntryPoints {
        const char *name;
        int index;
    } EntryPoints;

    typedef struct ShaderModule {
        const char *name;
        const char *path;
        const void *data;
        int size;
    } ShaderModule;

    typedef struct SlangGlobalSessionWrapper SlangGlobalSessionWrapper;
    typedef struct SlangSessionWrapper SlangSessionWrapper;
    typedef struct SlangLinkedWrapper SlangLinkedWrapper;
    typedef struct SlangModuleWrapper SlangModuleWrapper;
    typedef struct SlangEntryPointWrapper SlangEntryPointWrapper;
    typedef struct SlangProgramWrapper SlangProgramWrapper;

    typedef struct SlangString *(*LoadFileFunction)(char const *path);
    typedef int (*ListFilesFunction)(const char *path, void *callback, void *userData);
    typedef char* (*CombinePathsFunction)(const char *path1, const char *path2, int* outPathSize);
    typedef int (*CheckIsDirectoryFunction)(const char *path);

    EXPORT_CODE SlangGlobalSessionWrapper *ap_createGlobalSession(void);

    EXPORT_CODE void ap_destroyGlobalSession(SlangGlobalSessionWrapper *session);

    EXPORT_CODE SlangSessionWrapper *ap_createSession(SlangGlobalSessionWrapper *globalSession,
                                          LoadFileFunction file, ListFilesFunction lister, CombinePathsFunction combiner, CheckIsDirectoryFunction checker);

    EXPORT_CODE void ap_destroySession(SlangSessionWrapper *session);

    EXPORT_CODE SlangModuleWrapper *ap_loadModule(SlangSessionWrapper *session, char *path,
                                      const void **onError);

    EXPORT_CODE SlangModuleWrapper *ap_loadModuleFromCode(SlangSessionWrapper *session, char *name, char *path, char *code,
                                              const void **onError);

    EXPORT_CODE SlangModuleWrapper *ap_loadModuleFromIRBlob(SlangSessionWrapper *session, char *name, char *path, void *blob, int size,
                                              const void **onError);

    EXPORT_CODE int ap_isModuleValid(SlangSessionWrapper *sesson, char* name, void *blob, int size);

    EXPORT_CODE void ap_onLoadFile(int isFolder, char *name, void *callback, void *userData);

    EXPORT_CODE SlangEntryPointWrapper *ap_findEntryPoint(SlangModuleWrapper *module,
                                              char *path);

    EXPORT_CODE int ap_getEntryPointCount(SlangModuleWrapper *module);

    EXPORT_CODE void ap_getEntryPoints(SlangModuleWrapper* module, EntryPoints* entries);

    EXPORT_CODE SlangProgramWrapper *ap_compileProgram(SlangSessionWrapper *session, void **entryPoints,
                                           void **modules, int moduleCount, int entryPointCount, const void **onError);

    EXPORT_CODE SlangLinkedWrapper* ap_linkProgram(SlangProgramWrapper *program,
                               const void **onError);

    EXPORT_CODE const void* ap_getEntryPointCode(SlangLinkedWrapper* linked, int targetIndex, int* outSize, const void **onError);

    EXPORT_CODE int ap_getVariableCount(SlangSessionWrapper *session,
                            SlangProgramWrapper *program,
                            const void **onError);

    EXPORT_CODE void ap_getVariables(SlangSessionWrapper *session,
                         SlangProgramWrapper *program,
                         ShaderVariable *data);

    EXPORT_CODE void ap_freeSegment(ShaderVariable *data, int variableCount);

    EXPORT_CODE int ap_getModuleCount(SlangSessionWrapper *session);

    EXPORT_CODE void ap_getModules(SlangSessionWrapper *session, ShaderModule *data);

    EXPORT_CODE void ap_freeModules(int count, ShaderModule *data);
#ifdef __cplusplus
}
#endif

#endif /* SLANG_JAVA_SLANGJAVA_H */
