//! Validation-only host. No production backup, launcher, observer, or transfer code.
use std::sync::atomic::{AtomicBool, Ordering};
use tauri::{Manager, WebviewUrl, WebviewWindow};
struct State {
    allow_exit: AtomicBool,
}
fn local(w: &WebviewWindow) -> Result<(), String> {
    let u = w.url().map_err(|e| e.to_string())?;
    if w.app_handle().config().identifier != "com.gelb2.heavymach.desktop.validation"
        || w.label() != "validation"
        || !((u.scheme() == "tauri" && u.host_str() == Some("localhost"))
            || (u.scheme() == "http" && u.host_str() == Some("tauri.localhost")))
    {
        return Err("Local validation window required".into());
    }
    Ok(())
}
#[tauri::command]
fn validation_report(w: WebviewWindow, report: String) -> Result<(), String> {
    local(&w)?;
    if report.len() > 2_000_000 {
        return Err("Report too large".into());
    }
    let mut parsed: serde_json::Value = serde_json::from_str(&report).map_err(|e| e.to_string())?;
    if !parsed.is_object() {
        return Err("Report object required".into());
    }
    parsed["host_app_id"] = serde_json::Value::String(w.app_handle().config().identifier.clone());
    parsed["host_feature"] = serde_json::Value::String("validation".into());
    let dir = w
        .app_handle()
        .path()
        .app_data_dir()
        .map_err(|e| e.to_string())?
        .join("validation-reports");
    std::fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    let stamp = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_millis();
    let bytes = serde_json::to_vec_pretty(&parsed).map_err(|e| e.to_string())?;
    std::fs::write(dir.join(format!("run-{stamp}.json")), &bytes).map_err(|e| e.to_string())?;
    std::fs::write(dir.join("latest.json"), &bytes).map_err(|e| e.to_string())
}
#[tauri::command]
fn validation_finish_close(w: WebviewWindow, state: tauri::State<State>) -> Result<(), String> {
    local(&w)?;
    state.allow_exit.store(true, Ordering::SeqCst);
    w.app_handle().exit(0);
    Ok(())
}
fn close(w: &WebviewWindow) {
    let _ = w.eval("window.__hmValidationHost?.requestClose()");
}
pub fn run() {
    let app = tauri::Builder::default()
        .plugin(tauri_plugin_single_instance::init(|app, _, _| {
            if let Some(w) = app.get_webview_window("validation") {
                let _ = w.set_focus();
            }
        }))
        .manage(State {
            allow_exit: AtomicBool::new(false),
        })
        .invoke_handler(tauri::generate_handler![
            validation_report,
            validation_finish_close
        ])
        .setup(|app| {
            if app.config().identifier != "com.gelb2.heavymach.desktop.validation" {
                return Err("Validation feature requires the validation app identifier".into());
            }
            use tauri::menu::{MenuBuilder, MenuItemBuilder, SubmenuBuilder};
            let quit = MenuItemBuilder::with_id("validation-quit", "Quit Heavy Mach Validation")
                .accelerator("CmdOrCtrl+Q")
                .build(app)?;
            let run = MenuItemBuilder::with_id("validation-run", "Run Unit and Weapon Checks")
                .accelerator("CmdOrCtrl+Shift+V")
                .build(app)?;
            let menu = SubmenuBuilder::new(app, "Heavy Mach Validation")
                .hide()
                .hide_others()
                .show_all()
                .separator()
                .item(&run)
                .item(&quit)
                .build()?;
            app.set_menu(MenuBuilder::new(app).item(&menu).build()?)?;
            app.on_menu_event(|app, event| {
                if let Some(w) = app.get_webview_window("validation") {
                    match event.id().as_ref() {
                        "validation-quit" => close(&w),
                        "validation-run" => {
                            let _ = w.eval("window.__hmValidationHost?.run()");
                        }
                        _ => {}
                    }
                }
            });
            let autorun = std::env::var("HEAVYMACH_VALIDATION_RUN").as_deref() == Ok("1");
            let script = format!(
                "window.__hmValidationAutorun={};\n{}",
                autorun,
                include_str!("../../frontend/validation.js")
            );
            let window = tauri::WebviewWindowBuilder::new(
                app,
                "validation",
                WebviewUrl::App("index.html".into()),
            )
            .title("Heavy Mach 1 · Validation")
            .inner_size(1200., 800.)
            .min_inner_size(640., 360.)
            .initialization_script(script)
            .on_new_window(|_, _| tauri::webview::NewWindowResponse::Deny)
            .on_navigation(|u| {
                (u.scheme() == "tauri" && u.host_str() == Some("localhost"))
                    || (u.scheme() == "http" && u.host_str() == Some("tauri.localhost"))
            });
            #[cfg(target_os = "macos")]
            let window = window.data_store_identifier(*b"HeavyMachValV001");
            let window = window.build()?;
            let w = window.clone();
            window.on_window_event(move |event| {
                if let tauri::WindowEvent::CloseRequested { api, .. } = event {
                    api.prevent_close();
                    close(&w);
                }
            });
            Ok(())
        })
        .build(tauri::generate_context!())
        .expect("Create isolated Heavy Mach Validation");
    app.run(|app, event| {
        if let tauri::RunEvent::ExitRequested { api, .. } = event {
            if !app.state::<State>().allow_exit.load(Ordering::SeqCst) {
                api.prevent_exit();
                if let Some(w) = app.get_webview_window("validation") {
                    close(&w);
                }
            }
        }
    });
}
