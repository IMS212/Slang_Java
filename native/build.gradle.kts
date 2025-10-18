import org.gradle.nativeplatform.platform.internal.DefaultNativePlatform

plugins {
    id("cpp-library")
}

group = "org.example"
version = "1.0-SNAPSHOT"

library {
    baseName = "slang-java"
}

repositories {
    mavenCentral()
}

dependencies {
}

var os: OperatingSystem? = DefaultNativePlatform.getCurrentOperatingSystem()

tasks.withType<CppCompile>().configureEach {
    includes {
        rootDir.parentFile.resolve("slang").resolve("include")
    }

    if (os?.isWindows ?: false) {
        compilerArgs.addAll(listOf("/std:c++20", "/MD", "/O2", "/Gy", "/Gw"))
    }

}


tasks.withType<LinkSharedLibrary>().configureEach {
    // add the path to libslang.so
    if (os?.isWindows ?: false) {
        linkerArgs.addAll(
            listOf(
                "/OPT:REF",
                "/OPT:ICF",
                rootDir.resolve("slang.lib").absolutePath
            )
        )
    } else {
        val libDir = rootDir.absolutePath
        linkerArgs.addAll(
            listOf(
                "-O3",
                "-L$libDir",
                "-lslang",
                "-Wl,--enable-new-dtags",
                "-Wl,-rpath,\$ORIGIN"
            )
        )
    }

}