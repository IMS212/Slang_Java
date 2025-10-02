plugins {
    id("java")
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

dependencies {
    testImplementation(platform("org.junit:junit-bom:5.10.0"))
    testImplementation("org.junit.jupiter:junit-jupiter")

    implementation(platform("org.lwjgl:lwjgl-bom:$lwjglVersion"))

    implementation("org.lwjgl", "lwjgl")
    implementation("org.lwjgl", "lwjgl-assimp")
    implementation("org.lwjgl", "lwjgl-glfw")
    implementation("org.lwjgl", "lwjgl-openal")
    implementation("org.lwjgl", "lwjgl-opengl")
    implementation("org.lwjgl", "lwjgl-stb")
    implementation("org.lwjgl", "lwjgl-tinyfd")

    implementation ("org.lwjgl", "lwjgl", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-assimp", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-glfw", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-openal", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-opengl", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-stb", classifier = lwjglNatives)
    implementation ("org.lwjgl", "lwjgl-tinyfd", classifier = lwjglNatives)

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
    repositories {
        mavenLocal()
    }

    publications {
        create<MavenPublication>("maven") {
            groupId = project.group as String
            artifactId = rootProject.name + "-" + project.name
            version = version

            from(components["java"])
        }
    }
}