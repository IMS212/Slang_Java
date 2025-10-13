plugins {
    id("java-library")
    id("de.infolektuell.jextract") version "0.5.0"
    id("maven-publish")

}

group = "net.irisshaders"
version = "1.0-SNAPSHOT"
val lwjglVersion = "3.4.0-SNAPSHOT"
val lwjglNatives = "natives-linux"
repositories {
    mavenCentral()
    maven("https://central.sonatype.com/repository/maven-snapshots")
}
// for local runs only; never published/consumable
val localRun by configurations.creating {
    isCanBeConsumed = false
    isCanBeResolved = true
    isVisible = false
}
dependencies {
    testImplementation(platform("org.junit:junit-bom:5.10.0"))
    testImplementation("org.junit.jupiter:junit-jupiter")


    compileOnly(platform("org.lwjgl:lwjgl-bom:$lwjglVersion"))
    compileOnly("org.lwjgl:lwjgl")
    compileOnly("org.lwjgl:lwjgl-assimp")
    compileOnly("org.lwjgl:lwjgl-glfw")
    compileOnly("org.lwjgl:lwjgl-openal")
    compileOnly("org.lwjgl:lwjgl-opengl")
    compileOnly("org.lwjgl:lwjgl-stb")
    compileOnly("org.lwjgl:lwjgl-tinyfd")

    // natives only for local runs (not published)
    localRun(platform("org.lwjgl:lwjgl-bom:$lwjglVersion"))
    localRun ("org.lwjgl", "lwjgl", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-assimp", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-glfw", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-openal", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-opengl", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-stb", classifier = lwjglNatives)
    localRun ("org.lwjgl", "lwjgl-tinyfd", classifier = lwjglNatives)

}

tasks.test {
    useJUnitPlatform()
}
java {
    toolchain {
        languageVersion = JavaLanguageVersion.of(25) // Set your desired Java version (e.g., 8, 11, 17, 21)
    }
}
jextract.generator {
    local = layout.projectDirectory.dir("/home/ims/Downloads/jextract-22/jextract-22/")
    javaLanguageVersion = JavaLanguageVersion.of(25)
}
jextract.libraries {
    // The native BASS audio library.
    val slang by registering {
        header = layout.projectDirectory.file("native/src/main/public/slangJava.h")
        headerClassName = "SlangJava"
        targetPackage = "net.irisshaders.slang.internal"
        useSystemLoadLibrary = true
        libraries.add("slang")
        // Make your public headers folder searchable for Jextract
        includes.add(layout.projectDirectory.dir("native/src/main/public"))

    }

    sourceSets {
        // For KMP you want something like jvmMain
        main {
            jextract.libraries.addLater(slang)
        }
    }
}


publishing {
    publications {
        create<MavenPublication>("maven") {
            from(components["java"])
        }
    }
}