fn main() {
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(
        tauri_build::AppManifest::new().commands(&[
            "backup_status",
            "transfer_file",
            "backup_write",
            "finish_close",
            "close_error",
            "diagnostics_write",
            "restore_status",
            "export_save",
            "import_save",
            "restore_begin",
            "restore_finish",
        ]),
    ))
    .expect("Tauri build configuration");
}
