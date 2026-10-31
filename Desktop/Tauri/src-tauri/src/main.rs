#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
mod backup;
mod recovery;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, Mutex,
};
use tauri::{Manager, WebviewUrl, WebviewWindow};
use tauri_plugin_dialog::DialogExt;
struct State {
    writes: Arc<Mutex<()>>,
    allow_exit: AtomicBool,
}
fn local(w: &WebviewWindow) -> Result<(), String> {
    let u = w.url().map_err(|e| e.to_string())?;
    if w.label() != "main"
        || !((u.scheme() == "tauri" && u.host_str() == Some("localhost"))
            || (u.scheme() == "http" && u.host_str() == Some("tauri.localhost")))
    {
        return Err("Local game window required".into());
    }
    Ok(())
}
fn launcher(w: &WebviewWindow) -> Result<std::path::PathBuf, String> {
    local(w)?;
    if !matches!(
        w.url().map_err(|e| e.to_string())?.path(),
        "" | "/" | "/index.html"
    ) {
        return Err("Open the launcher to manage saves".into());
    }
    w.app_handle()
        .path()
        .app_data_dir()
        .map_err(|e| e.to_string())
}
#[tauri::command]
fn restore_status(w: WebviewWindow) -> Result<Option<recovery::Pending>, String> {
    recovery::pending(&launcher(&w)?)
}
#[tauri::command]
async fn export_save(w: WebviewWindow, snapshot: backup::Snapshot) -> Result<bool, String> {
    launcher(&w)?;
    snapshot.validate()?;
    let app = w.app_handle().clone();
    tauri::async_runtime::spawn_blocking(move || {
        let Some(file) = app
            .dialog()
            .file()
            .add_filter("Heavy Mach save", &["json"])
            .set_file_name("HeavyMach-save.json")
            .blocking_save_file()
        else {
            return Ok(false);
        };
        backup::export(&file.into_path().map_err(|e| e.to_string())?, snapshot)?;
        Ok(true)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
async fn import_save(w: WebviewWindow) -> Result<Option<backup::Envelope>, String> {
    launcher(&w)?;
    let app = w.app_handle().clone();
    tauri::async_runtime::spawn_blocking(move || {
        let Some(file) = app
            .dialog()
            .file()
            .add_filter("Heavy Mach save", &["json"])
            .blocking_pick_file()
        else {
            return Ok(None);
        };
        Ok(Some(backup::read(
            &file.into_path().map_err(|e| e.to_string())?,
        )?))
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
async fn restore_begin(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    chosen: backup::Envelope,
    current: backup::Snapshot,
) -> Result<recovery::Pending, String> {
    let root = launcher(&w)?;
    let lock = state.writes.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let _guard = lock.lock().map_err(|e| e.to_string())?;
        recovery::begin(&root, chosen, current)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
async fn restore_finish(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    current: backup::Snapshot,
) -> Result<(), String> {
    let root = launcher(&w)?;
    let lock = state.writes.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let _guard = lock.lock().map_err(|e| e.to_string())?;
        recovery::finish(&root, current)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
fn backup_status(w: WebviewWindow) -> Result<backup::Status, String> {
    local(&w)?;
    backup::status(
        &w.app_handle()
            .path()
            .app_data_dir()
            .map_err(|e| e.to_string())?
            .join("saves"),
    )
}
#[tauri::command]
async fn backup_write(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    snapshot: backup::Snapshot,
) -> Result<bool, String> {
    local(&w)?;
    let lock = state.writes.clone();
    let dir = w
        .app_handle()
        .path()
        .app_data_dir()
        .map_err(|e| e.to_string())?
        .join("saves");
    tauri::async_runtime::spawn_blocking(move || {
        let _guard = lock.lock().map_err(|e| e.to_string())?;
        if recovery::pending(dir.parent().ok_or("Missing data folder")?)?.is_some() {
            return Err("Finish the pending restore before playing".into());
        }
        backup::write(&dir, snapshot)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
fn finish_close(w: WebviewWindow, state: tauri::State<State>) -> Result<(), String> {
    local(&w)?;
    state.allow_exit.store(true, Ordering::SeqCst);
    w.app_handle().exit(0);
    Ok(())
}
#[tauri::command]
fn close_error(w: WebviewWindow, message: String) -> Result<(), String> {
    local(&w)?;
    let detail: String = message.chars().take(500).collect();
    w.dialog().message(format!("Could not finish the backup. The game is still open.\n{detail}\nRetry Quit after resolving the error."))
        .title("Heavy Mach Desktop").show(|_| {});
    Ok(())
}
#[tauri::command]
fn diagnostics_write(w: WebviewWindow, report: String) -> Result<(), String> {
    local(&w)?;
    if report.len() > 65536 {
        return Err("Report too large".into());
    }
    let _: serde_json::Value = serde_json::from_str(&report).map_err(|e| e.to_string())?;
    let dir = w
        .app_handle()
        .path()
        .app_data_dir()
        .map_err(|e| e.to_string())?
        .join("diagnostics");
    std::fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    let stamp = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_millis();
    std::fs::write(dir.join(format!("frames-{stamp}.json")), report).map_err(|e| e.to_string())
}
fn request_close(w: &WebviewWindow) {
    if let Err(e) = w.eval("window.__hmDesktop?.requestClose()") {
        w.dialog()
            .message(format!("Could not request quit. Retry.\n{e}"))
            .title("Heavy Mach Desktop")
            .show(|_| {});
    }
}

fn main() {
    let app = tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_single_instance::init(|app, _, _| {
            if let Some(w) = app.get_webview_window("main") {
                let _ = w.set_focus();
            }
        }))
        .manage(State {
            writes: Arc::new(Mutex::new(())),
            allow_exit: AtomicBool::new(false),
        })
        .invoke_handler(tauri::generate_handler![
            backup_status,
            backup_write,
            finish_close,
            close_error,
            diagnostics_write,
            restore_status,
            export_save,
            import_save,
            restore_begin,
            restore_finish
        ])
        .setup(|app| {
            use tauri::menu::{MenuBuilder, MenuItemBuilder, SubmenuBuilder};
            // Native macOS predefined Quit sends terminate: directly. Route Cmd-Q
            // through our completed-save backup before calling app.exit().
            let quit = MenuItemBuilder::with_id("quit-after-backup", "Quit Heavy Mach Desktop")
                .accelerator("CmdOrCtrl+Q")
                .build(app)?;
            let app_menu = SubmenuBuilder::new(app, "Heavy Mach Desktop")
                .hide()
                .hide_others()
                .show_all()
                .separator()
                .item(&quit)
                .build()?;
            let edit_menu = SubmenuBuilder::new(app, "Edit")
                .undo()
                .redo()
                .separator()
                .cut()
                .copy()
                .paste()
                .select_all()
                .build()?;
            let measure =
                MenuItemBuilder::with_id("measure-frames", "Record 15 Seconds of Frame Timing")
                    .accelerator("CmdOrCtrl+Shift+P")
                    .build(app)?;
            let diagnostics_menu = SubmenuBuilder::new(app, "Diagnostics")
                .item(&measure)
                .build()?;
            let window_menu = SubmenuBuilder::new(app, "Window")
                .minimize()
                .maximize()
                .build()?;
            app.set_menu(
                MenuBuilder::new(app)
                    .items(&[&app_menu, &edit_menu, &window_menu, &diagnostics_menu])
                    .build()?,
            )?;
            app.on_menu_event(|app, event| {
                if event.id().as_ref() == "measure-frames" {
                    if let Some(w) = app.get_webview_window("main") {
                        let _ = w.eval("window.__hmMeasure?.()");
                    }
                }
                if event.id().as_ref() == "quit-after-backup" {
                    if let Some(w) = app.get_webview_window("main") {
                        request_close(&w);
                    }
                }
            });
            let script = concat!(
                include_str!("../../frontend/storage.js"),
                "\n",
                include_str!("../../frontend/observer.js"),
                "\n",
                include_str!("../../frontend/diagnostics.js")
            );
            let window =
                tauri::WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
                    .title("Heavy Mach Desktop")
                    .inner_size(1200., 800.)
                    .min_inner_size(640., 360.)
                    .initialization_script(script)
                    .on_new_window(|_, _| tauri::webview::NewWindowResponse::Deny)
                    .on_navigation(|u| {
                        (u.scheme() == "tauri" && u.host_str() == Some("localhost"))
                            || (u.scheme() == "http" && u.host_str() == Some("tauri.localhost"))
                    });
            #[cfg(target_os = "macos")]
            let window = window.data_store_identifier(*b"HeavyMachDeskV01");
            let window = window.build()?;
            let w = window.clone();
            window.on_window_event(move |event| {
                if let tauri::WindowEvent::CloseRequested { api, .. } = event {
                    api.prevent_close();
                    request_close(&w);
                }
            });
            Ok(())
        })
        .build(tauri::generate_context!())
        .expect("Create Heavy Mach Desktop");
    app.run(|app, event| {
        if let tauri::RunEvent::ExitRequested { api, .. } = event {
            if !app.state::<State>().allow_exit.load(Ordering::SeqCst) {
                api.prevent_exit();
                if let Some(w) = app.get_webview_window("main") {
                    request_close(&w);
                }
            }
        }
    });
}
