package net.irisshaders.slang;

@FunctionalInterface
public interface FileFinderFunction {
    void onFileFound(boolean isDirectory, String path);
}
