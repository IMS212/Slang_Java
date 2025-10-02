#ifndef SLANG_JAVA_SLANGJAVA_H
#define SLANG_JAVA_SLANGJAVA_H

#ifdef __cplusplus
extern "C" {
#endif

    struct SlangString {
        char *data;
        int size;
    };

    typedef struct ShaderVariable {
        char *name;
        int binding;
        int set;
        int type;
    } ShaderVariable;

    typedef struct ShaderModule {
        const char *name;
        const char *path;
        const void *data;
        int size;
    } ShaderModule;

    typedef struct SlangGlobalSessionWrapper SlangGlobalSessionWrapper;
    typedef struct SlangSessionWrapper SlangSessionWrapper;
    typedef struct SlangModuleWrapper SlangModuleWrapper;
    typedef struct SlangEntryPointWrapper SlangEntryPointWrapper;
    typedef struct SlangProgramWrapper SlangProgramWrapper;

    typedef struct SlangString *(*LoadFileFunction)(char const *path);

    typedef int (*ListFilesFunction)(const char *path, void *callback, void *userData);

    SlangGlobalSessionWrapper *ap_createGlobalSession(void);

    void ap_destroyGlobalSession(SlangGlobalSessionWrapper *session);

    SlangSessionWrapper *ap_createSession(SlangGlobalSessionWrapper *globalSession,
                                          LoadFileFunction file, ListFilesFunction lister);

    void ap_destroySession(SlangSessionWrapper *session);

    SlangModuleWrapper *ap_loadModule(SlangSessionWrapper *session, char *path,
                                      const void **onError);

    SlangModuleWrapper *ap_loadModuleFromCode(SlangSessionWrapper *session, char *name, char *path, char *code,
                                              const void **onError);

    void ap_onLoadFile(int isFolder, char *name, void *callback, void *userData);

    SlangEntryPointWrapper *ap_findEntryPoint(SlangModuleWrapper *module,
                                              char *path);

    SlangProgramWrapper *ap_compileProgram(SlangSessionWrapper *session,
                                           SlangEntryPointWrapper *entryPoint,
                                           void **modules, int size, const void **onError);

    void const *ap_linkProgram(SlangSessionWrapper *session,
                               SlangProgramWrapper *program,
                               const void **onError);

    int ap_getVariableCount(SlangSessionWrapper *session,
                            SlangProgramWrapper *program,
                            const void **onError);

    void ap_getVariables(SlangSessionWrapper *session,
                         SlangProgramWrapper *program,
                         ShaderVariable *data);

    void ap_freeSegment(ShaderVariable *data, int variableCount);

    int ap_getModuleCount(SlangSessionWrapper *session);

    void ap_getModules(SlangSessionWrapper *session, ShaderModule *data);

    void ap_freeModules(int count, ShaderModule *data);
#ifdef __cplusplus
}
#endif

#endif /* SLANG_JAVA_SLANGJAVA_H */
