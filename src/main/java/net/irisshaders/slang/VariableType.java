package net.irisshaders.slang;

public enum VariableType {
    UBO,
    SSBO,
    SAMPLER,
    IMAGE,
    UNIFORM;

    public static VariableType fromInt(int type) {
        // slang::ParameterCategory
        if (type == 2) return UBO;
        if (type == 7) return SAMPLER;
        if (type == 4) return IMAGE;
        return SSBO;
    }
}
