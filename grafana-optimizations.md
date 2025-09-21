# Grafana Optimizations — Practical Notes

These notes summarize how I optimize Grafana dashboards backed by InfluxDB (Flux). The goals are faster panels, lower query cost, and clearer maintenance.

---

## Principles

1) **Filter early, aggregate early**  
   Push `range()` and `filter()` to the top; do `aggregateWindow()` as soon as practical to reduce series size.

2) **Select only what you need**  
   Keep just the relevant fields/measurements/tags; avoid wide `group()` unless required.

3) **Consistent variables**  
   Use dashboard variables (`$host`, `$branch`, `$interval`) instead of hard-coded values.

4) **Avoid redundant transforms**  
   Remove no-op `map()` and repeated `group()`. Keep pipeline linear and readable.

5) **Name your outputs**  
   Use `yield(name: "mean")` or similar so downstream/inspectors are clear.

---

## Before → After (Flux)

**Before (slow / noisy):**
```flux
from(bucket: "telegraf")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "cpu")
  |> filter(fn: (r) => r._field == "usage_system")
  |> filter(fn: (r) => r.cpu == "cpu-total")
  |> group(columns: ["_measurement","_field","host","cpu"])
  |> aggregateWindow(every: 10s, fn: mean, createEmpty: true)
  |> group()            // redundant regroup
  |> fill(usePrevious: true)
  |> yield()
