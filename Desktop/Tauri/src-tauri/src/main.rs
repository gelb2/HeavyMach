#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
#[cfg(not(feature = "validation"))]
mod backup;
#[cfg(not(feature = "validation"))]
mod normal;
#[cfg(not(feature = "validation"))]
mod recovery;
#[cfg(feature = "validation")]
mod validation;
fn main() {
    #[cfg(not(feature = "validation"))]
    normal::run();
    #[cfg(feature = "validation")]
    validation::run();
}
