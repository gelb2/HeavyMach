//! Explicit launcher-only restore. Keep old files and a durable pending intent.
use crate::backup::{self, Envelope, Snapshot};
use serde::{Deserialize, Serialize};
use std::{fs, path::Path};
#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Pending {
    pub id: String,
    pub chosen: Envelope,
}
pub fn pending(root: &Path) -> Result<Option<Pending>, String> {
    let path = root.join("restore-pending.json");
    if !path.exists() {
        return Ok(None);
    }
    if fs::metadata(&path).map_err(|e| e.to_string())?.len() > 262144 {
        return Err("Restore intent too large".into());
    }
    let bytes = fs::read(path).map_err(|e| e.to_string())?;
    if bytes.len() > 262144 {
        return Err("Restore intent too large".into());
    }
    let p: Pending = serde_json::from_slice(&bytes).map_err(|e| e.to_string())?;
    if p.id.is_empty() || p.id.len() > 32 || !p.id.bytes().all(|c| c.is_ascii_digit()) {
        return Err("Invalid restore identifier".into());
    }
    backup::validate_envelope(&p.chosen)?;
    Ok(Some(p))
}
pub fn begin(root: &Path, chosen: Envelope, current: Snapshot) -> Result<Pending, String> {
    if pending(root)?.is_some() {
        return Err("Finish the pending restore first".into());
    }
    backup::validate_envelope(&chosen)?;
    current.validate()?;
    let id = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map_err(|e| e.to_string())?
        .as_nanos()
        .to_string();
    let dir = root.join("recovery").join(&id);
    fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    backup::export(&dir.join("before-import.json"), current)?;
    backup::export(&dir.join("selected.json"), chosen.snapshot.clone())?;
    let p = Pending { id, chosen };
    use std::io::Write;
    let mut tmp = tempfile::NamedTempFile::new_in(root).map_err(|e| e.to_string())?;
    tmp.write_all(&serde_json::to_vec(&p).map_err(|e| e.to_string())?)
        .and_then(|_| tmp.as_file().sync_all())
        .map_err(|e| e.to_string())?;
    tmp.persist_noclobber(root.join("restore-pending.json"))
        .map_err(|e| e.to_string())?;
    backup::sync_directory(&dir)?;
    backup::sync_directory(&root.join("recovery"))?;
    backup::sync_directory(root)?;
    Ok(p)
}
pub fn finish(root: &Path, current: Snapshot) -> Result<(), String> {
    let p = pending(root)?.ok_or("No pending restore")?;
    current.validate()?;
    // Sort records because IndexedDB returns keys in lexical order.
    let mut a = current;
    let mut b = p.chosen.snapshot.clone();
    a.records.sort_by(|x, y| x.path.cmp(&y.path));
    b.records.sort_by(|x, y| x.path.cmp(&y.path));
    if a != b {
        return Err("Restored data does not match the selected backup".into());
    }
    let dir = root.join("recovery").join(&p.id);
    let saves = root.join("saves");
    let previous = dir.join("previous-saves");
    // A crash after renaming preserves both the pending intent and the previous directory.
    if !previous.exists() && saves.exists() {
        fs::rename(&saves, &previous).map_err(|e| e.to_string())?;
    }
    backup::sync_directory(&dir)?;
    backup::sync_directory(root)?;
    backup::write(&saves, p.chosen.snapshot)?;
    fs::remove_file(root.join("restore-pending.json")).map_err(|e| e.to_string())?;
    backup::sync_directory(root)?;
    Ok(())
}
#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::BTreeMap;
    fn sample(n: u8) -> Snapshot {
        Snapshot {
            format: 1,
            records: vec![backup::Record {
                path: "/persist/HvM.dat".into(),
                timestamp: 10,
                mode: 0o100666,
                contents: Some(vec![n; 3624]),
            }],
            settings: BTreeMap::from([
                ("HeavyMach.AimModeV1".into(), Some("1".into())),
                ("HeavyMach.KeyboardBindingsV1".into(), None),
            ]),
        }
    }
    #[test]
    fn recover_corruption_preserves_all_old_data() {
        let t = tempfile::tempdir().unwrap();
        let r = t.path();
        backup::write(&r.join("saves"), sample(1)).unwrap();
        fs::write(r.join("saves/broken.json"), "bad").unwrap();
        let p = begin(r, backup::envelope(sample(2)).unwrap(), sample(3)).unwrap();
        assert!(finish(r, sample(1)).is_err());
        assert!(pending(r).unwrap().is_some());
        finish(r, sample(2)).unwrap();
        let d = r.join("recovery").join(p.id);
        assert_eq!(
            backup::read(&d.join("before-import.json"))
                .unwrap()
                .snapshot,
            sample(3)
        );
        assert_eq!(
            fs::read(d.join("previous-saves/broken.json")).unwrap(),
            b"bad"
        );
        assert_eq!(
            backup::status(&r.join("saves"))
                .unwrap()
                .latest
                .unwrap()
                .snapshot,
            sample(2)
        );
        assert!(pending(r).unwrap().is_none());
    }
    #[test]
    fn interrupted_directory_swap_resumes() {
        let t = tempfile::tempdir().unwrap();
        let r = t.path();
        backup::write(&r.join("saves"), sample(1)).unwrap();
        let p = begin(r, backup::envelope(sample(2)).unwrap(), sample(1)).unwrap();
        fs::rename(
            r.join("saves"),
            r.join("recovery").join(p.id).join("previous-saves"),
        )
        .unwrap();
        finish(r, sample(2)).unwrap();
        assert_eq!(
            backup::status(&r.join("saves"))
                .unwrap()
                .latest
                .unwrap()
                .snapshot,
            sample(2)
        );
    }
    #[test]
    fn mismatched_digest_does_not_create_intent() {
        let t = tempfile::tempdir().unwrap();
        let mut e = backup::envelope(sample(2)).unwrap();
        e.digest = "bad".into();
        assert!(begin(t.path(), e, sample(1)).is_err());
        assert!(pending(t.path()).unwrap().is_none());
    }
}
