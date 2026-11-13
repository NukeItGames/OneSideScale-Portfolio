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
| Hosted GitHub Actions | NOT RUN: repository creation blocked by integration permissions |
| Publication | BLOCKED: GitHub createRepository returned Resource not accessible by integration |
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

## Publication handoff

The source and fresh local Git history are prepared. GitHub rejected creation of
`NukeItGames/OneSideScale-Portfolio` with `Resource not accessible by integration
(createRepository)`. No public push was attempted and no live public URL is
claimed. The intended URL is https://github.com/NukeItGames/OneSideScale-Portfolio.

The owner must create an **empty public repository** with that exact name,
without initializing README, license, or gitignore. Then push the prepared
`main` branch, verify its tree and public visibility, inspect Actions results,
and recheck the original repository's private visibility/refs. This document's
publication/CI status should be updated with those actual results afterward.
