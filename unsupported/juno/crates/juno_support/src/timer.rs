/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::fmt::Display;
use std::fmt::Formatter;
use std::time::Duration;
use std::time::Instant;

type Mark = (&'static str, Duration);

/// A convenience utility to measure execution time of sections.
#[derive(Debug)]
pub struct Timer {
    start_time: Instant,
    last_update: Duration,
    marks: Vec<Mark>,
}

impl Default for Timer {
    fn default() -> Self {
        Timer {
            start_time: Instant::now(),
            last_update: Default::default(),
            marks: vec![],
        }
    }
}

impl Timer {
    pub fn new() -> Self {
        Default::default()
    }

    /// Record the duration of the just completed section.
    pub fn mark(&mut self, name: &'static str) {
        let new_upd = self.start_time.elapsed();
        let duration = new_upd - self.last_update;
        self.last_update = new_upd;
        self.marks.push((name, duration));
    }
}

impl Display for Timer {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        // In "alternate" mode, pretty print the result in aligned table form.
        // In "normal mode", just dump the marks using debug formatting (JSON-like).
        if f.alternate() {
            let max_name_width = self
                .marks
                .iter()
                .fold(0usize, |accum, m| accum.max(m.0.len()));

            // Determine the number of decimal places and scale.
            let mut dec = 1;
            let mut scale = 0;
            let mut dur = self.last_update.as_secs_f64();
            for i in 0..=3 {
                scale = i;
                if dur >= 1.0 {
                    dec = f64::log10(dur).trunc() as usize + 1;
                    break;
                } else if i < 3 {
                    dur *= 1e3;
                }
            }

            const SCALES: [&str; 4] = ["s", "ms", "us", "ns"];

            let mult = f64::powf(1e3, scale as f64);

            let mut fmt = |m: &Mark| {
                writeln!(
                    f,
                    "{0:1$}: {2:>3$.4$} {5}",
                    m.0,
                    max_name_width,
                    m.1.as_secs_f64() * mult,
                    dec + 4,
                    3,
                    SCALES[scale]
                )
            };

            for m in &self.marks {
                fmt(m)?;
            }
            fmt(&("Total", self.last_update))?;
        } else {
            let mut map = f.debug_map();
            for m in &self.marks {
                map.entry(&m.0, &m.1);
            }
            map.entry(&"Total", &self.last_update);
            map.finish()?;
        }

        Ok(())
    }
}
