fn main() {
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(
        tauri_build::AppManifest::new().commands(&[
            "backup_status",
            "backup_write",
            "finish_close",
        ]),
    ))
    .expect("Tauri build configuration");
}
