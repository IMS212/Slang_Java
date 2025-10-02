plugins {
    id("cpp-library")
}

group = "org.example"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

dependencies {
}


tasks.withType<CppCompile>().configureEach {
    includes {
        "/home/ims/slang/include"
    }

}


tasks.withType<LinkSharedLibrary>().configureEach {
    // add the path to libslang.so
    linkerArgs.addAll(
        listOf(
            "/home/ims/slang/cmake-build-debug/Debug/lib/libslang.so"
        )
    )
}