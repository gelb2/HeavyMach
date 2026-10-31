//! OS-file snapshots of already committed browser data. No game save calls.
use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};
use std::{
    collections::{BTreeMap, BTreeSet},
    fs::{self, File, OpenOptions},
    io::{Read, Write},
    path::Path,
};

const MAX_BYTES: u64 = 256 * 1024;
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq)]
#[serde(deny_unknown_fields)]
pub struct Record {
    pub path: String,
    pub timestamp: i64,
    pub mode: u32,
    pub contents: Option<Vec<u8>>,
}
#[derive(Clone, Debug, Serialize, Deserialize, PartialEq)]
#[serde(deny_unknown_fields)]
pub struct Snapshot {
    pub format: u32,
    pub records: Vec<Record>,
    pub settings: BTreeMap<String, Option<String>>,
}
#[derive(Clone, Debug, Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
pub struct Envelope {
    pub sequence: u64,
    pub digest: String,
    pub snapshot: Snapshot,
}
#[derive(Debug, Serialize)]
pub struct Status {
    pub latest: Option<Envelope>,
    pub invalid: Vec<String>,
}
fn hash(bytes: &[u8]) -> String {
    format!("{:x}", Sha256::digest(bytes))
}
impl Snapshot {
    pub fn validate(&self) -> Result<(), String> {
        if self.format != 1 || self.records.len() > 3 {
            return Err("Unsupported backup format".into());
        }
        let mut paths = BTreeSet::new();
        for r in &self.records {
            if !paths.insert(&r.path) || r.timestamp < 0 {
                return Err("Invalid or duplicate record".into());
            }
            match r.path.as_str() {
                "/persist" if r.mode & 0o170000 == 0o040000 && r.contents.is_none() => (),
                "/persist/HvM.dat" if r.mode & 0o170000 == 0o100000 => {
                    if r.contents.as_ref().map(Vec::len) != Some(3624) {
                        return Err("Invalid HvM.dat length".into());
                    }
                }
                "/persist/HUDLayoutV1-web.json" if r.mode & 0o170000 == 0o100000 => {
                    let bytes = r.contents.as_ref().ok_or("Missing HUD data")?;
                    if bytes.len() > 32768 {
                        return Err("HUD data too large".into());
                    }
                    let v: serde_json::Value =
                        serde_json::from_slice(bytes).map_err(|e| e.to_string())?;
                    let positions = v["positions"].as_array().ok_or("Missing HUD positions")?;
                    if v["version"] != 1 || positions.len() > 10 {
                        return Err("Unsupported HUD layout".into());
                    }
                    for p in positions {
                        if p.is_null() {
                            continue;
                        }
                        let a = p.as_array().ok_or("Invalid HUD position")?;
                        if a.len() != 2
                            || a.iter()
                                .any(|n| n.as_f64().is_none_or(|x| !(0.0..=1.0).contains(&x)))
                        {
                            return Err("HUD position out of range".into());
                        }
                    }
                }
                _ => return Err("Unknown backup path or file type".into()),
            }
        }
        if self.settings.len() != 2
            || !self.settings.contains_key("HeavyMach.AimModeV1")
            || !self.settings.contains_key("HeavyMach.KeyboardBindingsV1")
        {
            return Err("Unexpected settings keys".into());
        }
        if let Some(v) = &self.settings["HeavyMach.AimModeV1"] {
            if v != "0" && v != "1" {
                return Err("Invalid aim mode".into());
            }
        }
        if let Some(v) = &self.settings["HeavyMach.KeyboardBindingsV1"] {
            let codes: Result<Vec<u16>, _> = v.split(',').map(str::parse).collect();
            let codes = codes.map_err(|_| "Invalid keyboard bindings")?;
            let mut seen = BTreeSet::new();
            if !(15..=16).contains(&codes.len())
                || codes
                    .iter()
                    .any(|c| *c == 0 || *c >= 256 || *c == 0xe3 || *c == 0xe7 || !seen.insert(*c))
            {
                return Err("Invalid keyboard bindings".into());
            }
        }
        Ok(())
    }
    fn content_digest(&self) -> String {
        let mut copy = self.clone();
        copy.records.sort_by(|a, b| a.path.cmp(&b.path));
        for r in &mut copy.records {
            r.timestamp = 0;
        }
        hash(&serde_json::to_vec(&copy).expect("Serializable snapshot"))
    }
}
fn read(path: &Path) -> Result<Envelope, String> {
    let f = File::open(path).map_err(|e| e.to_string())?;
    if f.metadata().map_err(|e| e.to_string())?.len() > MAX_BYTES {
        return Err("Backup too large".into());
    }
    let mut bytes = Vec::new();
    f.take(MAX_BYTES + 1)
        .read_to_end(&mut bytes)
        .map_err(|e| e.to_string())?;
    if bytes.len() as u64 > MAX_BYTES {
        return Err("Backup too large".into());
    }
    let e: Envelope = serde_json::from_slice(&bytes).map_err(|e| e.to_string())?;
    e.snapshot.validate()?;
    if hash(&serde_json::to_vec(&e.snapshot).map_err(|e| e.to_string())?) != e.digest {
        return Err("Backup checksum mismatch".into());
    }
    Ok(e)
}
pub fn status(dir: &Path) -> Result<Status, String> {
    if !dir.exists() {
        return Ok(Status {
            latest: None,
            invalid: vec![],
        });
    }
    let mut latest: Option<Envelope> = None;
    let mut invalid = Vec::new();
    for entry in fs::read_dir(dir).map_err(|e| e.to_string())? {
        let path = entry.map_err(|e| e.to_string())?.path();
        if path.extension().is_none_or(|e| e != "json") {
            continue;
        }
        match read(&path) {
            Ok(e) => {
                if latest.as_ref().is_none_or(|old| e.sequence > old.sequence) {
                    latest = Some(e);
                }
            }
            Err(_) => invalid.push(path.file_name().unwrap().to_string_lossy().into_owned()),
        }
    }
    Ok(Status { latest, invalid })
}
pub fn write(dir: &Path, snapshot: Snapshot) -> Result<bool, String> {
    write_at(dir, snapshot, None)
}
fn write_at(dir: &Path, snapshot: Snapshot, fail: Option<&str>) -> Result<bool, String> {
    snapshot.validate()?;
    let old = status(dir)?;
    // An unreadable existing backup requires a recovery choice, not automatic replacement.
    if !old.invalid.is_empty() {
        return Err("Damaged backup files need review before writing".into());
    }
    if let Some(e) = &old.latest {
        if e.snapshot.content_digest() == snapshot.content_digest() {
            return Ok(false);
        }
    }
    fs::create_dir_all(dir).map_err(|e| e.to_string())?;
    let sequence = old.latest.as_ref().map_or(Ok(1), |e| {
        e.sequence.checked_add(1).ok_or("Backup sequence exhausted")
    })?;
    let envelope = Envelope {
        sequence,
        digest: hash(&serde_json::to_vec(&snapshot).map_err(|e| e.to_string())?),
        snapshot,
    };
    let bytes = serde_json::to_vec(&envelope).map_err(|e| e.to_string())?;
    if bytes.len() as u64 > MAX_BYTES {
        return Err("Backup too large".into());
    }
    let tmp = dir.join(format!("snapshot-{sequence:020}.pending"));
    // A stale incomplete file is never interpreted as a completed backup.
    let mut f = OpenOptions::new()
        .write(true)
        .create(true)
        .truncate(true)
        .open(&tmp)
        .map_err(|e| e.to_string())?;
    f.write_all(&bytes)
        .and_then(|_| f.sync_all())
        .map_err(|e| e.to_string())?;
    drop(f);
    read(&tmp)?;
    if fail == Some("before-commit") {
        return Err("Injected interruption before commit".into());
    }
    let final_path = dir.join(format!("snapshot-{sequence:020}.json"));
    if final_path.exists() {
        return Err("Snapshot already exists".into());
    }
    fs::rename(&tmp, &final_path).map_err(|e| e.to_string())?;
    #[cfg(unix)]
    File::open(dir)
        .and_then(|f| f.sync_all())
        .map_err(|e| e.to_string())?;
    if fail == Some("after-commit") {
        return Err("Injected interruption after commit".into());
    }
    // Keep two complete generations. Invalid and incomplete files remain for investigation.
    for entry in fs::read_dir(dir).map_err(|e| e.to_string())? {
        let p = entry.map_err(|e| e.to_string())?.path();
        if p.extension().is_some_and(|e| e == "json") {
            if let Ok(e) = read(&p) {
                if e.sequence + 1 < sequence {
                    fs::remove_file(p).map_err(|e| e.to_string())?;
                }
            }
        }
    }
    Ok(true)
}
#[cfg(test)]
mod tests {
    use super::*;
    fn sample(n: u8) -> Snapshot {
        Snapshot {
            format: 1,
            records: vec![Record {
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
    fn no_write_for_same_content() {
        let d = tempfile::tempdir().unwrap();
        assert!(write(d.path(), sample(1)).unwrap());
        let mut s = sample(1);
        s.records[0].timestamp = 99;
        assert!(!write(d.path(), s).unwrap());
        assert_eq!(fs::read_dir(d.path()).unwrap().count(), 1);
    }
    #[test]
    fn retain_two_generations() {
        let d = tempfile::tempdir().unwrap();
        for n in 0..4 {
            write(d.path(), sample(n)).unwrap();
        }
        assert_eq!(fs::read_dir(d.path()).unwrap().count(), 2);
        assert_eq!(
            status(d.path()).unwrap().latest.unwrap().snapshot,
            sample(3)
        );
    }
    #[test]
    fn interruption_before_commit_preserves_previous() {
        let d = tempfile::tempdir().unwrap();
        write(d.path(), sample(1)).unwrap();
        assert!(write_at(d.path(), sample(2), Some("before-commit")).is_err());
        assert_eq!(
            status(d.path()).unwrap().latest.unwrap().snapshot,
            sample(1)
        );
        assert!(write(d.path(), sample(2)).unwrap());
    }
    #[test]
    fn interruption_after_commit_recovers_new() {
        let d = tempfile::tempdir().unwrap();
        write(d.path(), sample(1)).unwrap();
        assert!(write_at(d.path(), sample(2), Some("after-commit")).is_err());
        assert_eq!(
            status(d.path()).unwrap().latest.unwrap().snapshot,
            sample(2)
        );
    }
    #[test]
    fn corrupt_newest_preserves_previous_and_blocks_write() {
        let d = tempfile::tempdir().unwrap();
        write(d.path(), sample(1)).unwrap();
        write(d.path(), sample(2)).unwrap();
        fs::write(
            d.path().join("snapshot-00000000000000000002.json"),
            b"partial",
        )
        .unwrap();
        let s = status(d.path()).unwrap();
        assert_eq!(s.invalid.len(), 1);
        assert_eq!(s.latest.unwrap().snapshot, sample(1));
        assert!(write(d.path(), sample(3)).is_err());
    }
    #[test]
    fn unknown_path_and_bad_length_rejected() {
        let d = tempfile::tempdir().unwrap();
        let mut s = sample(1);
        s.records[0].path = "../../save".into();
        assert!(write(d.path(), s).is_err());
        let mut s = sample(1);
        s.records[0].contents = Some(vec![0; 3]);
        assert!(write(d.path(), s).is_err());
        assert_eq!(fs::read_dir(d.path()).unwrap().count(), 0);
    }
    #[test]
    fn checksum_detects_modified_contents() {
        let d = tempfile::tempdir().unwrap();
        write(d.path(), sample(1)).unwrap();
        let p = d.path().join("snapshot-00000000000000000001.json");
        let mut e = read(&p).unwrap();
        e.snapshot.records[0].contents.as_mut().unwrap()[0] = 8;
        fs::write(&p, serde_json::to_vec(&e).unwrap()).unwrap();
        assert!(read(&p).is_err());
    }
    #[test]
    fn write_failure_retains_existing() {
        let d = tempfile::tempdir().unwrap();
        write(d.path(), sample(1)).unwrap();
        fs::create_dir(d.path().join("snapshot-00000000000000000002.pending")).unwrap();
        assert!(write(d.path(), sample(2)).is_err());
        assert_eq!(
            status(d.path()).unwrap().latest.unwrap().snapshot,
            sample(1)
        );
    }
}
