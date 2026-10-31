#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
mod backup;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, Mutex,
};
use tauri::{Manager, WebviewUrl, WebviewWindow};
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
fn request_close(w: &WebviewWindow) {
    let _ = w.eval("window.__hmDesktop?.requestClose()");
}
fn main() {
    let app = tauri::Builder::default()
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
            finish_close
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
            let window_menu = SubmenuBuilder::new(app, "Window")
                .minimize()
                .maximize()
                .build()?;
            app.set_menu(
                MenuBuilder::new(app)
                    .items(&[&app_menu, &edit_menu, &window_menu])
                    .build()?,
            )?;
            app.on_menu_event(|app, event| {
                if event.id().as_ref() == "quit-after-backup" {
                    if let Some(w) = app.get_webview_window("main") {
                        request_close(&w);
                    }
                }
            });
            let script = concat!(
                include_str!("../../frontend/storage.js"),
                "\n",
                include_str!("../../frontend/observer.js")
            );
            let window =
                tauri::WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
                    .title("Heavy Mach Desktop")
                    .inner_size(1280., 720.)
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
