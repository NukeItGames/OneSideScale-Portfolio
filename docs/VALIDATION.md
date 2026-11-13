# Validation results

Initial preparation: November 13, 2026, Linux x86_64.

| Check | Result |
| --- | --- |
| Source/history review | Eight commits, 41 unique historical blobs, complete current tree reviewed |
| Gitleaks source snapshot | PASS, zero findings |
| Gitleaks original history and all historical blobs | PASS, zero findings |
| Original C++/headers/build rules preserved | PASS, all 12 Source files byte-identical |
| Repository checks and host generation | PASS locally: 26 files; descriptor, links, icon reproducibility, independent host, overwrite/path refusal |
| Final snapshot secret scan | PASS, Gitleaks 8.30.1, zero findings |
| Fresh portfolio history secret scan | PASS, zero findings; new root with no imported source commits |
| CI workflow syntax | PASS, Actionlint 1.7.7; upstream action tag hashes verified |
| Hosted GitHub Actions | PASS on prepared root: Linux checks, Windows checks, full-history secret scan |
| Publication | PASS: prepared root pushed to public main without force; all 26 files independently cloned and byte-verified |
| Original repository safety | PASS: private visibility retained; complete remote ref set unchanged |
| Unreal plugin / Editor target compilation | NOT RUN: no Unreal installation or engine toolchain |
| Unreal automation | NOT RUN: all five definitions preserved; no editor available |
| Actual scaling tool in viewport | NOT RUN: no editor available |
| Screenshots and recorded demonstration | Unavailable: source contains only toolbar artwork |

No Unreal runtime validation is claimed. A filesystem search for UnrealEditor,
RunUAT, and installed engine directories found no engine. A system C++ compiler
alone cannot compile the Unreal module without engine headers, generated UHT
code, libraries, and UnrealBuildTool. Unreal was not downloaded or redistributed.

The source-declared target is UE 5.8.2. Windows build instructions and an
independent C++ host are provided, but they have not been exercised with Unreal.
The interaction test needs a real Level Editor viewport; it is not covered by
engine-free CI or a commandlet. See EDITOR_TESTING.md for exact test names,
automation/report instructions, manual steps, and a genuine capture procedure.

The host generator copies source/resources and supplies an empty game module,
Game/Editor targets, and an enabled plugin descriptor. Engine-free checks verify
the generated layout, byte-identical copied source, overwrite refusal, and
refusal to create the host inside the repository. This verifies generation,
not UnrealBuildTool acceptance or editor behavior.

Compatibility on other Unreal releases, operating systems, headless runners,
Blueprint-only installation, binary packaging, and all real viewport interactions
remains unverified. Native toolbar integration is especially version-sensitive.

## Publication verification

Published November 13, 2026:
https://github.com/NukeItGames/OneSideScale-Portfolio

The owner created the empty public repository after the integration's initial
creation request was denied. The existing reviewed root commit
`97c4573c5833438e62fb5c7427f10db348f9317e` was pushed to `main` without force,
without rebuilding or importing private history. Its tree is
`04c493f64c2e14d4f71621fa90d03d388ad2e88f`. An independent clone matched every
one of the 26 prepared files byte-for-byte, including the complete Source tree,
README, MIT license, standalone build instructions, and Actions workflow.

[Actions run 43354000063](https://github.com/NukeItGames/OneSideScale-Portfolio/actions/runs/43354000063)
completed successfully on that root: engine-free checks on `ubuntu-latest` and
`windows-latest`, plus the full-history Gitleaks scan. These are repository/host
generation and security checks, not Unreal compilation or editor tests.

GitHub authentication identified NukeItGames and repository metadata reported
push/admin access. Git push succeeded. Description and topics updates were each
denied with HTTP 403, `Resource not accessible by integration`; the integration's
repository-settings permissions are narrower than the user's role.

The original repository's private visibility and complete remote ref set were
rechecked against the initial audit before publication. No original-repository
write was performed. A documentation-only follow-up records these results;
all 12 original Source files remain unchanged. No new Unreal compilation,
automation, viewport test, screenshot, or recording was performed.
