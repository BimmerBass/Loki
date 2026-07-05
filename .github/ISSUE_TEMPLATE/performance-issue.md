---
name: Performance Issue
about: Improve the performance of a part of Loki
title: ''
labels: performance
assignees: BimmerBass

---

## Goal

Describe the performance improvement, measurement, or decision this issue should produce.

## Affected Area

- [ ] Move generation
- [ ] Make/unmake
- [ ] Board/state representation
- [ ] Search
- [ ] Evaluation
- [ ] Transposition table
- [ ] UCI/protocol
- [ ] Testing/benchmarking
- [ ] Other:

## Baseline

Record the current measured behavior.

```text
Command:
Position(s):
Result:
Environment:
Commit:
```

## Hypothesis

What do we think is slow, expensive, over-allocated, or unnecessary?

## Experiment

What change or measurement will test the hypothesis?

## Success Criteria

- [ ] Baseline measurement is recorded
- [ ] Experiment result is recorded
- [ ] Correctness is preserved
- [ ] Performance change is large enough to justify the code change
- [ ] Decision is documented

## Validation

- [ ] Self-play
- [ ] Unit tests
- [ ] Perft/regression tests
- [ ] Benchmark comparison
- [ ] Profiling output
- [ ] Manual engine run

## Result

Fill this in before closing the issue.

```text
Before:
After:
Delta:
Decision:
```

## Notes

Links, profiles, flamegraphs, benchmark logs, or follow-up ideas.
