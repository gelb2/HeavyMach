// Production storage modules against isolated OS failures. Never opens app data.
#[path = "../src/backup.rs"]
mod backup;
#[path = "../src/recovery.rs"]
mod recovery;
use std::{collections::BTreeMap, fs, process::Command, thread, time::Duration};
fn sample(n: u8) -> backup::Snapshot {
    backup::Snapshot {
        format: 1,
        records: vec![backup::Record {
            path: "/persist/HvM.dat".into(),
            timestamp: 10,
            mode: 0o100666,
            contents: Some(vec![n; 3624]),
        }],
        settings: BTreeMap::from([
            ("HeavyMach.AimModeV1".into(), Some("0".into())),
            ("HeavyMach.KeyboardBindingsV1".into(), None),
        ]),
    }
}
#[test]
#[cfg(unix)]
fn permission_denied_retains_save() {
    use std::os::unix::fs::PermissionsExt;
    let t = tempfile::tempdir().unwrap();
    let dir = t.path().join("saves");
    backup::write(&dir, sample(1)).unwrap();
    fs::set_permissions(&dir, fs::Permissions::from_mode(0o500)).unwrap();
    let result = backup::write(&dir, sample(2));
    fs::set_permissions(&dir, fs::Permissions::from_mode(0o700)).unwrap();
    assert!(
        result
            .as_ref()
            .is_err_and(|s| s.contains("Permission denied")),
        "{result:?}"
    );
    assert_eq!(
        backup::status(&dir).unwrap().latest.unwrap().snapshot,
        sample(1)
    );
}
#[test]
#[ignore = "spawned by killed_writer_retains_complete_generation"]
fn child_writer() {
    let root = std::env::var("HM_FAULT_ROOT").unwrap();
    let dir = std::path::Path::new(&root);
    for n in 2..=200 {
        backup::write(dir, sample(n)).unwrap();
        fs::write(dir.join("ready"), [n]).unwrap();
        thread::sleep(Duration::from_millis(2));
    }
    panic!("Parent did not kill child");
}
#[test]
fn killed_writer_retains_complete_generation() {
    let t = tempfile::tempdir().unwrap();
    backup::write(t.path(), sample(1)).unwrap();
    let mut child = Command::new(std::env::current_exe().unwrap())
        .args(["--ignored", "--exact", "child_writer"])
        .env("HM_FAULT_ROOT", t.path())
        .stdout(std::process::Stdio::null())
        .spawn()
        .unwrap();
    for _ in 0..200 {
        if t.path().join("ready").exists() {
            break;
        }
        thread::sleep(Duration::from_millis(5));
    }
    assert!(t.path().join("ready").exists());
    child.kill().unwrap();
    let status = child.wait().unwrap();
    assert!(!status.success());
    let saved = backup::status(t.path()).unwrap();
    assert!(saved.invalid.is_empty());
    assert!(saved.latest.unwrap().sequence >= 2);
    backup::write(t.path(), sample(201)).unwrap();
    assert_eq!(
        backup::status(t.path()).unwrap().latest.unwrap().snapshot,
        sample(201)
    );
}
#[test]
#[ignore = "requires dedicated mounted test disk in HM_FULL_DISK"]
fn disk_full_retains_save() {
    use std::io::Write;
    let root = std::env::var("HM_FULL_DISK").unwrap();
    let root = std::path::Path::new(&root);
    assert!(root.join("heavymach-test-volume").is_file());
    let dir = root.join("saves");
    backup::write(&dir, sample(1)).unwrap();
    let mut f = fs::File::create(root.join("fill")).unwrap();
    let fill_error = loop {
        if let Err(e) = f.write_all(&[0u8; 4096]) {
            break e;
        }
    };
    assert_eq!(fill_error.raw_os_error(), Some(28));
    drop(f);
    let result = backup::write(&dir, sample(2));
    assert!(
        result.as_ref().is_err_and(|s| s.contains("No space left")),
        "{result:?}"
    );
    assert_eq!(
        backup::status(&dir).unwrap().latest.unwrap().snapshot,
        sample(1)
    );
    fs::remove_file(root.join("fill")).unwrap();
    backup::write(&dir, sample(2)).unwrap();
    assert_eq!(
        backup::status(&dir).unwrap().latest.unwrap().snapshot,
        sample(2)
    );
}
