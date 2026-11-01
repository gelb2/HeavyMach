plugins { id("com.android.application"); id("org.jetbrains.kotlin.android") }
val prepareAssets by tasks.registering(Exec::class) {
    workingDir(rootProject.projectDir)
    commandLine("python3", "tools/prepare_assets.py", layout.buildDirectory.dir("generated/gameAssets").get().asFile.absolutePath)
    inputs.files(fileTree("../../Resources/OriginalImages"),fileTree("../../Resources/RawData"),fileTree("../../Resources/Audio"),fileTree("../../Art/Extensions"))
    outputs.dir(layout.buildDirectory.dir("generated/gameAssets"))
}
android {
    namespace = "com.gelb2.heavymach"
    compileSdk = 36
    ndkVersion = "26.3.11579264"
    defaultConfig {
        applicationId = "com.gelb2.heavymach.restored"
        minSdk = 26; targetSdk = 36; versionCode = 1; versionName = "0.16-android-dev"
        ndk { abiFilters += listOf("arm64-v8a") }
        externalNativeBuild { cmake { arguments += listOf("-DANDROID_STL=c++_static", "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON", "-DCMAKE_POLICY_VERSION_MINIMUM=3.5"); targets += "heavymach" } }
    }
    flavorDimensions += "edition"
    productFlavors {
        create("normal") { dimension = "edition"; buildConfigField("boolean", "CHEAT", "false"); resValue("string", "app_name", "Heavy Mach 1"); externalNativeBuild { cmake { arguments += "-DHM_ANDROID_VALIDATION=OFF" } } }
        create("cheat") { dimension = "edition"; applicationId = "com.gelb2.heavymach.validation"; buildConfigField("boolean", "CHEAT", "true"); resValue("string", "app_name", "HM Validation"); externalNativeBuild { cmake { arguments += "-DHM_ANDROID_VALIDATION=ON" } } }
    }
    buildTypes { getByName("release") { isMinifyEnabled = false } }
    buildFeatures { buildConfig = true }
    externalNativeBuild { cmake { path = file("../cmake/CMakeLists.txt"); version = "3.22.1" } }
    sourceSets.getByName("main").assets.srcDir(layout.buildDirectory.dir("generated/gameAssets"))
    compileOptions { sourceCompatibility = JavaVersion.VERSION_17; targetCompatibility = JavaVersion.VERSION_17 }
    kotlinOptions { jvmTarget = "17" }
    packaging { jniLibs { useLegacyPackaging = false } }
}
tasks.named("preBuild").configure { dependsOn(prepareAssets) }
